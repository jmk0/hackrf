#include <libhackrf/hackrf.h>
#include <iostream>
#include <cmath>
#include <unistd.h>

using namespace std;

const uint64_t xmitFreq = 144750000; // in Hz
const int freq = 3125; // also in Hz, but divided..
const int divider = 100;

/** This class is just a dummy to be used as a context object for the
 * sample block callback function.  One potential use we might have
 * for it is to store pre-generated signal buffers to be copied as
 * appropriate into the transfer buffer in the callback.
 * Right now it doesn't do anything. */
class Tx1Context
{
public:
   Tx1Context();
   unsigned long foo;
};

Tx1Context ::
Tx1Context()
      : foo(666)
{
}

/** This is the callback function for hackrf_start_tx.  It doesn't do
 * a lot just yet.
 * @param[in] transfer The data structure with all the data transfer
 *   buffers and what-not to be transmitted.
 * @return -1 to immediately stop transmitting as we're not doing
 *   anything to fill the transmit buffer. */
int sampleBlockCB(hackrf_transfer* transfer)
{
   cerr << "buffer_length=" << transfer->buffer_length << endl
        << "valid_length=" << transfer->valid_length << endl;
   return -1;
}


int main(int argc, char *argv[])
{
   int rv;
   hackrf_device *device;
      // initialize the library
   rv = hackrf_init();
   if (rv != HACKRF_SUCCESS)
   {
      cerr << "hackrf_init failed: " << hackrf_error_name((hackrf_error)rv)
           << endl;
   }
   else
   {
      cout << "hackrf_init success" << endl;
   }
      // get a list of all the attached hackrf devices
   hackrf_device_list_t* devices = hackrf_device_list();
   if (devices == nullptr)
   {
      cerr << "hackrf_device_list failed" << endl;
   }
   else
   {
      cout << "hackrf_device_list:" << endl;
      for (int i = 0; i < devices->devicecount; i++)
      {
         cout << "  S/N " << devices->serial_numbers[i] << endl;
            // Just pick the first one since I'm unlikely to get a 2nd
            // any time soon.
         if (i == 0)
         {
            rv = hackrf_device_list_open(devices, i, &device);
            if (rv != HACKRF_SUCCESS)
            {
               cerr << "hackrf_device_list_open failed: "
                    << hackrf_error_name((hackrf_error)rv) << endl;
            }
            else
            {
               cout << "hackrf_device_list_open success" << endl;
            }
         }
      }
      hackrf_device_list_free(devices);
   }
      // set the data sample rate
      // undocumented *sigh*
   rv = hackrf_set_sample_rate_manual(device, freq, divider);
   if (rv != HACKRF_SUCCESS)
   {
      cerr << "hackrf_set_sample_rate_manual failed: "
           << hackrf_error_name((hackrf_error)rv) << endl;
   }
   else
   {
      cout << "hackrf_set_sample_rate_manual success" << endl;
   }
      // set the transmit (and probably rx as well) frequency
      // also undocumented.
   rv = hackrf_set_freq(device, xmitFreq);
   if (rv != HACKRF_SUCCESS)
   {
      cerr << "hackrf_set_freq failed: "
           << hackrf_error_name((hackrf_error)rv) << endl;
   }
   else
   {
      cout << "hackrf_set_freq success" << endl;
   }
      // Theoretically, start transmitting.  Doesn't really do much
      // since the callback immediately returns a failure.
   Tx1Context context;
   rv = hackrf_start_tx(device, sampleBlockCB, &context);
   if (rv != HACKRF_SUCCESS)
   {
      cerr << "hackrf_start_tx failed: "
           << hackrf_error_name((hackrf_error)rv) << endl;
   }
   else
   {
      cout << "hackrf_start_tx success" << endl;
   }
      // Wait a little bit before terminating.  Normally right here
      // you would probably check hackrf_is_streaming and any signal
      // handlers.
   sleep(1);
      // Close down the library and device.
   rv = hackrf_exit();
   if (rv != HACKRF_SUCCESS)
   {
      cerr << "hackrf_exit failed: "
           << hackrf_error_name((hackrf_error)rv) << endl;
   }
   else
   {
      cout << "hackrf_exit success" << endl;
   }      
   return 0;
}
