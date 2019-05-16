using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace GUI
{
    public partial class FormHost : Form
    {

        private OptionsForm childForm;
        public FormHost()
        {
            InitializeComponent();

            childForm = new OptionsForm();
            childForm.MdiParent = this;
            childForm.Show();
            childForm.Dock = DockStyle.Fill;
            tbl.Controls.Add(childForm);
            tbl.SetCellPosition(childForm, new TableLayoutPanelCellPosition(0, 0));
        }

        
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
         
            Console.SetWindowSize(Console.LargestWindowWidth, Console.LargestWindowHeight);
            Console.SetBufferSize(Console.LargestWindowWidth, Console.LargestWindowHeight);

            this.BeginInvoke(new Action(() =>
            {
                var windows = GetRootWindowsOfProcess(System.Diagnostics.Process.GetCurrentProcess().Id);
                foreach (var wnd in windows)
                {
                    string wndName = GetWindowText(wnd);
                    if (wndName == "ROC")
                    {
                        FormWrapperControl uc = new FormWrapperControl(wnd);
                        tbl.Controls.Add(uc);
                        uc.Dock = DockStyle.Fill;
                        tbl.SetCellPosition(uc, new TableLayoutPanelCellPosition(0, 1));
                    }
                    else if (wndName == "Set")
                    {
                        FormWrapperControl uc = new FormWrapperControl(wnd);
                        split.Panel1.Controls.Add(uc);
                        uc.Dock = DockStyle.Fill;
                        //tbl.SetCellPosition(uc, new TableLayoutPanelCellPosition(1, 0));
                        //tbl.SetRowSpan(uc, 2);
                    }
                    else if ((wndName + "").ToLower().EndsWith(".exe"))
                    {
                        // console window
                        FormWrapperControl uc = new FormWrapperControl(wnd);
                        split.Panel2.Controls.Add(uc);
                        uc.Dock = DockStyle.Fill;
                    }
                }
            }));
        }


        private void ExitToolsStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            base.OnFormClosed(e);
            Environment.Exit(0);
        }







        List<IntPtr> GetRootWindowsOfProcess(int pid)
        {
            List<IntPtr> rootWindows = GetChildWindows(IntPtr.Zero);
            List<IntPtr> dsProcRootWindows = new List<IntPtr>();
            foreach (IntPtr hWnd in rootWindows)
            {
                uint lpdwProcessId;
                GetWindowThreadProcessId(hWnd, out lpdwProcessId);
                if (lpdwProcessId == pid)
                    dsProcRootWindows.Add(hWnd);
            }
            return dsProcRootWindows;
        }

        public static List<IntPtr> GetChildWindows(IntPtr parent)
        {
            List<IntPtr> result = new List<IntPtr>();
            GCHandle listHandle = GCHandle.Alloc(result);
            try
            {
                Win32Callback childProc = new Win32Callback(EnumWindow);
                EnumChildWindows(parent, childProc, GCHandle.ToIntPtr(listHandle));
            }
            finally
            {
                if (listHandle.IsAllocated)
                    listHandle.Free();
            }
            return result;
        }

        private static bool EnumWindow(IntPtr handle, IntPtr pointer)
        {
            GCHandle gch = GCHandle.FromIntPtr(pointer);
            List<IntPtr> list = gch.Target as List<IntPtr>;
            if (list == null)
            {
                throw new InvalidCastException("GCHandle Target could not be cast as List<IntPtr>");
            }
            list.Add(handle);
            //  You can modify this to check to see if you want to cancel the operation, then return a null here
            return true;
        }

        private string GetWindowText(IntPtr handle)
        {
            int capacity = GetWindowTextLength(new HandleRef(this, handle)) * 2;
            StringBuilder stringBuilder = new StringBuilder(capacity);
            GetWindowText(new HandleRef(this, handle), stringBuilder, stringBuilder.Capacity);
            return stringBuilder.ToString();
        }

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern int GetWindowTextLength(HandleRef hWnd);
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern int GetWindowText(HandleRef hWnd, StringBuilder lpString, int nMaxCount);

        [DllImport("user32.dll")]
        public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint lpdwProcessId);

        [DllImport("user32.Dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool EnumChildWindows(IntPtr parentHandle, Win32Callback callback, IntPtr lParam);

        public delegate bool Win32Callback(IntPtr hwnd, IntPtr lParam);

        private void openSetToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (FolderBrowserDialog dlg = new FolderBrowserDialog())
            {
                dlg.SelectedPath = Environment.CurrentDirectory;
                if(dlg.ShowDialog(this) == DialogResult.OK)
                {
                    childForm.LoadSet(dlg.SelectedPath, false);   
                }
            }
        }

        private void openVideoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog dlg = new OpenFileDialog())
            {
                if (dlg.ShowDialog(this) == DialogResult.OK)
                    childForm.LoadSet(dlg.FileName, true);
            }
        }

        private void processEntireVideoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog dlg = new OpenFileDialog())
            {
                if (dlg.ShowDialog(this) == DialogResult.OK)
                {
                    using (FolderBrowserDialog fdlg = new FolderBrowserDialog())
                    {
                        fdlg.Description = "Select output folder";
                        if(fdlg.ShowDialog(this) == DialogResult.OK)
                        {
                            childForm.ProcessEntireVideo(dlg.FileName, fdlg.SelectedPath);
                        }
                    }
                }
                    
            }
        }

        private void calculateSetPerformanceToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (FolderBrowserDialog dlg = new FolderBrowserDialog())
            {
                dlg.SelectedPath = Environment.CurrentDirectory;
                if (dlg.ShowDialog(this) == DialogResult.OK)
                {
                    childForm.CalculateSetPerformance(dlg.SelectedPath);
                }
            }

          
            
        }

        private void shortcutsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string msg = "This form is an MDI form that groups the forms from OpenCV together with Win32 api's. They still act like separate forms which is why you still need to focus the forms to trigger waitKey()." + Environment.NewLine + Environment.NewLine + "The following shortcuts are available on the main frame window: " + Environment.NewLine ;
            msg += "- T: Toggle between the input and annotated images returned from each classifier" + Environment.NewLine;
            msg += "- C: Toggle the output mask result or for the individual classifier (depending on which annotated image is visible)" + Environment.NewLine;
            msg += "- G: Shows the ground truth if there is one available for the current frame" + Environment.NewLine;
            msg += "- A: Toggles the extra text info and steering direction mask" + Environment.NewLine;
            msg += "- N: Goes to the next frame" + Environment.NewLine;

            MessageBox.Show(msg, "Shortcuts", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
    }
}
