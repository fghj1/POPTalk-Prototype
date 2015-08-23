using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace ServerManagementTool
{
    static class Program
    {
        /// <summary>
        /// 해당 응용 프로그램의 주 진입점입니다.
        /// </summary>
        [STAThread]
        static void Main()
        {
            MainForm MainWidnow = null;

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault( false );

            MainWidnow = new MainForm();
            if( null == MainWidnow )
            {
                System.Diagnostics.Debug.WriteLine( "Failed to create the MainForm type object." );
                return;
            }

            Application.Run( MainWidnow );
        }
    }
}
