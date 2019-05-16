using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace GUI
{
    public partial class FormWrapperControl : UserControl
    {

        const int GWL_STYLE = (-16);

        const UInt32 WS_POPUP = 0x80000000;
        const UInt32 WS_CHILD = 0x40000000;

        const UInt32 WS_VISIBLE = 0x10000000;

        [DllImport("user32.dll")]
        public static extern IntPtr SetParent(IntPtr hWndChild, IntPtr hWndNewParent);


        [DllImport("user32.dll")]
        public static extern int SetWindowLong(IntPtr hWnd, int nIndex, UInt32 dwNewLong);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern bool MoveWindow(IntPtr hWnd, int X, int Y, int nWidth, int nHeight, bool bRepaint);



        private IntPtr wnd;
        private bool hideBorder;

        public FormWrapperControl(IntPtr wnd, bool hideBorder = true)
        {
            InitializeComponent();
            this.wnd = wnd;
            this.hideBorder = hideBorder;
        }
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            this.BeginInvoke(new Action(() =>
            {
                SetParent(wnd, this.Handle);
                if (!hideBorder)
                    SetWindowLong(wnd, GWL_STYLE, WS_VISIBLE);
                System.Threading.Thread.Sleep(10);
                MoveWindow(wnd, 0, 0, ClientRectangle.Width, ClientRectangle.Height, true);
            }));
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);

            MoveWindow(wnd, 0, 0, ClientRectangle.Width, ClientRectangle.Height, true);
        }
    }
}
