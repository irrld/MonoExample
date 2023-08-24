using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace Hello
{
    public class Main
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static string GetName();

        public static string GetHelloWorld()
        {
            return "Hello " + GetName() + "!";
        }
    }
}
