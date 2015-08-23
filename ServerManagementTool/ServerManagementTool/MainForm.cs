using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace ServerManagementTool
{
    public partial class MainForm : Form
    {
        public Network PeerNetwork = new Network();

        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_Load( object sender, EventArgs e )
        {
            Visible = false;

            LoginForm LoginWindow = null;

            LoginWindow = new LoginForm();
            if( null == LoginWindow )
            {
                System.Diagnostics.Debug.WriteLine( "Failed to create the LoginForm type object." );
                return;
            }

            if( DialogResult.OK == LoginWindow.ShowDialog( this ) )
                Visible = true;
            else
                Close();
        }

        private void MainForm_FormClosed( object sender, FormClosedEventArgs e )
        {
            // TODO: Network 관련 자원을 해제할 것
            //       메인 창에서 X 버튼을 눌러 프로그램을 종료할 때 필요한 처리
            Application.Exit();
            // TODO: X 버튼 누르면 창만 안보인다.
            //       관련 자원들은 해제되지 못하고 살아있다.
            // TODO: Application.Exit()이 아닌 안전하게 종료하는 방법을 찾아야 한다.
        }
    }
}
