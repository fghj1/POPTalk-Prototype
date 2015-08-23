using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace ServerManagementTool
{
    public partial class LoginForm : Form
    {
        IPAddress IPADR = null;
        int Port = 0;
        string ID = "";
        string Password = "";
        MainForm MainWindow = null;

        public LoginForm()
        {
            InitializeComponent();
        }

        protected override bool ProcessCmdKey( ref Message msg, Keys keyData )
        {
            if( Keys.Escape == keyData )
            {
                DialogResult = DialogResult.Cancel;
                return true;
            }

            return base.ProcessCmdKey( ref msg, keyData );
        }

        // IP
        private void ipAddressControl1_Leave( object sender, EventArgs e )
        {
            if( IPAddress.TryParse( ipAddressControl1.Text, out IPADR ) )
                errorProvider1.Clear();
            else
            {
                errorProvider1.SetIconAlignment( ipAddressControl1, ErrorIconAlignment.MiddleLeft );
                errorProvider1.SetIconPadding( ipAddressControl1, 5 );
                errorProvider1.SetError( ipAddressControl1, Properties.Resources.LoginForm_errorProvider1_String1 );
            }
        }

        // Port
        private void maskedTextBox1_Leave( object sender, EventArgs e )
        {
            if( int.TryParse( maskedTextBox1.Text, out Port ) &&
                IPEndPoint.MinPort <= Port && Port <= IPEndPoint.MaxPort )
                errorProvider2.Clear();
            else
            {
                errorProvider2.SetIconAlignment( maskedTextBox1, ErrorIconAlignment.MiddleLeft );
                errorProvider2.SetIconPadding( maskedTextBox1, 5 );
                errorProvider2.SetError( maskedTextBox1, Properties.Resources.LoginForm_errorProvider2_String1 );
            }
        }

        // Remember IP and Port
        // TODO: 여기에 작업

        // ID
        private void textBox1_Leave( object sender, EventArgs e )
        {
            ID = Regex.Replace( textBox1.Text, Properties.Resources.LoginForm_RegExpPatt_String1, "" );  // 특수 문자 제거
            textBox1.Text = ID;

            if( "" != ID )
                errorProvider3.Clear();
            else
            {
                errorProvider3.SetIconAlignment( textBox1, ErrorIconAlignment.MiddleLeft );
                errorProvider3.SetIconPadding( textBox1, 5 );
                errorProvider3.SetError( textBox1, Properties.Resources.LoginForm_errorProvider3_String1 );
            }
        }

        // Password
        private void textBox2_Leave( object sender, EventArgs e )
        {
            Password = textBox2.Text;

            if( "" != Password )
                errorProvider4.Clear();
            else
            {
                errorProvider4.SetIconAlignment( textBox2, ErrorIconAlignment.MiddleLeft );
                errorProvider4.SetIconPadding( textBox2, 5 );
                errorProvider4.SetError( textBox2, Properties.Resources.LoginForm_errorProvider4_String1 );
            }
        }

        // Remember my ID
        // TODO: 여기에 작업

        // Hide characters
        private void checkBox3_CheckedChanged( object sender, EventArgs e )
        {
            if( true == checkBox3.Checked )
                textBox2.UseSystemPasswordChar = true;
            else
                textBox2.UseSystemPasswordChar = false;
        }

        // Login
        private void button1_Click( object sender, EventArgs e )
        {
            MainWindow = Owner as MainForm;
            int iResult = MainWindow.PeerNetwork.Connect( IPADR, Port );
            if( 0 == iResult )
                DialogResult = DialogResult.OK;
            else
            {
                string szMsg = Properties.Resources.LoginForm_MessageBox_String1 + " ErrorCode:" + iResult;
                MessageBox.Show( this, szMsg );
            }
        }

        // Cancel
        private void button2_Click( object sender, EventArgs e )
        {
            DialogResult = DialogResult.Cancel;
        }
    }
}
