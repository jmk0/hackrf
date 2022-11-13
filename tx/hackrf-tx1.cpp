#include <libhackrf/hackrf.h>
#include <iostream>
#include <cmath>
#include <unistd.h>
#include <signal.h>

using namespace std;

const uint64_t xmitFreq = 144750000; // in Hz
const int freq = 3125; // also in Hz, but divided..
const int divider = 100;
   // Sample rate in sps, when not using manual
   // 8 Msps is the minimum recommended for the hackrf one
const int sampRate = 8000000;
const int API_FAIL = 1; ///< Exit code for API failure.

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

/// Termination flag for signal handler.
bool timeToDie = false;
/// Caught signal.
int caughtSig = 0;

/// Signal handler to cleanly handle termination.
void signalHandler(int sig)
{
   timeToDie = true;
   caughtSig = sig;
}

/// Set up the signal handler.
void setupSigHandler()
{
   signal(SIGHUP, signalHandler);
   signal(SIGINT, signalHandler);
   signal(SIGTERM, signalHandler);
   signal(SIGQUIT, signalHandler);
   signal(SIGPIPE, signalHandler);
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
      return API_FAIL;
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
      return API_FAIL;
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
               return API_FAIL;
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
      //rv = hackrf_set_sample_rate_manual(device, freq, divider);
   rv = hackrf_set_sample_rate(device, sampRate);
   if (rv != HACKRF_SUCCESS)
   {
      cerr << "hackrf_set_sample_rate_manual failed: "
           << hackrf_error_name((hackrf_error)rv) << endl;
      return API_FAIL;
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
      return API_FAIL;
   }
   else
   {
      cout << "hackrf_set_freq success" << endl;
   }
      // set up the signal handler before we start transmitting
   setupSigHandler();
      // Theoretically, start transmitting.  Doesn't really do much
      // since the callback immediately returns a failure.
   Tx1Context context;
   rv = hackrf_start_tx(device, sampleBlockCB, &context);
   if (rv != HACKRF_SUCCESS)
   {
      cerr << "hackrf_start_tx failed: "
           << hackrf_error_name((hackrf_error)rv) << endl;
      return API_FAIL;
   }
   else
   {
      cout << "hackrf_start_tx success" << endl;
   }
      // Wait before terminating.
   while ((hackrf_is_streaming(device) == HACKRF_TRUE) && !timeToDie)
   {
      sleep(1);
   }
      // Put the termination message here and not in the signal
      // handler as certain operations like stream I/O are not
      // recommended in that context.
   if (timeToDie)
   {
      cerr << "Caught signal " << caughtSig << ", terminating" << endl;
   }
      // Close down the device.
   if (device != nullptr)
   {
      rv = hackrf_stop_tx(device);
      if (rv != HACKRF_SUCCESS)
      {
         cerr << "hackrf_stop_tx failed: "
              << hackrf_error_name((hackrf_error)rv) << endl;
         return API_FAIL;
      }
      else
      {
         cout << "hackrf_stop_tx success" << endl;
      }
      rv = hackrf_close(device);
      if (rv != HACKRF_SUCCESS)
      {
         cerr << "hackrf_close failed: "
              << hackrf_error_name((hackrf_error)rv) << endl;
         return API_FAIL;
      }
      else
      {
         cout << "hackrf_close success" << endl;
      }
   }
      // Close down the library.
   rv = hackrf_exit();
   if (rv != HACKRF_SUCCESS)
   {
      cerr << "hackrf_exit failed: "
           << hackrf_error_name((hackrf_error)rv) << endl;
      return API_FAIL;
   }
   else
   {
      cout << "hackrf_exit success" << endl;
   }
   return 0;
}
