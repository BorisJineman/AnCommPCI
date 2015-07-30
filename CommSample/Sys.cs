using System;
using System.Collections.Generic;
using System.Text;
using SharpPcap;

namespace CommSample
{
    public static class Sys
    {
        public static ICaptureDevice Dev;
        public static byte[] SenderMAC;
        public static byte[] ReceiverMAC;
        public static byte[] ProtocolType;

    }
}
