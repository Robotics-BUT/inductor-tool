using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace tester
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void chart1_Click(object sender, EventArgs e)
        {

        }

        UInt16 oldid = 0;
        double[] adc1 = new double[0x1000];
        double[] adc2 = new double[0x1000];
        double[] adc3 = new double[0x1000];

        double adc1_offset = 47932; // nula
        double adc2_offset = 47595;
        double adc3_offset = -255;

        double adc1_gain = -3805 * 2; // 1A
        double adc2_gain = 3616 * 2;
        double adc3_gain = 2320;

        double time_scale = 0.0238;



        private UInt16 ReadU16(System.IO.Stream stm)
        {
            byte[] buffer = new byte[2];
            stm.Read(buffer, 0, 2);
            return (UInt16)(buffer[0] + buffer[1] * 256);
        }

        private void port_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            while (port.BytesToRead > 8)
            {
                UInt16 id = ReadU16(port.BaseStream);

                if ((oldid + 1 != id) && (id != 1) && (oldid != 0xFFF))
                {
                    Trace.WriteLine("LOST");
                    oldid = 0;
                    continue;
                }

                oldid = id;

                adc1[id] = (ReadU16(port.BaseStream) - adc1_offset) / adc1_gain;
                adc2[id] = (ReadU16(port.BaseStream) - adc2_offset) / adc2_gain;
                adc3[id] = (ReadU16(port.BaseStream) - adc3_offset) / adc3_gain;
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            chart1.Series[0].Points.Clear();
            chart1.Series[1].Points.Clear();
            chart1.Series[2].Points.Clear();

            for (int i = 0; i < 0xFFF; i++)
            {
                chart1.Series[0].Points.AddXY(i * time_scale, 0);
                chart1.Series[1].Points.AddXY(i * time_scale, 0);
                chart1.Series[2].Points.AddXY(i * time_scale, 0);
            }

            port.Open();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < 0xFFF; i++)
            {
                chart1.Series[0].Points[i].SetValueY(adc1[i]);
                chart1.Series[1].Points[i].SetValueY(adc2[i]);
                chart1.Series[2].Points[i].SetValueY(adc3[i]);
            }

            chart1.ChartAreas[0].RecalculateAxesScale();
            chart1.Refresh();
        }

        private void numericUpDown1_ValueChanged(object sender, EventArgs e)
        {
            chart1.ChartAreas[0].CursorY.Position = (float)numericUpDown1.Value;
        }

        private void chart1_MouseMove(object sender, MouseEventArgs e)
        {
            Point mousePoint = new Point(e.X, e.Y);

            chart1.ChartAreas[0].CursorX.SetCursorPixelPosition(mousePoint, true);

            double curpos = chart1.ChartAreas[0].CursorX.Position;
            int curofs = (int)(curpos / time_scale);
            double i1 = adc1[curofs & 0xFFF];
            double i2 = adc2[curofs & 0xFFF];
            double vcc = adc3[curofs & 0xFFF];

            bCursorTime.Text = string.Format("{0:F2} us", curpos / 100);
            bCursorSample.Text = string.Format("0x{0:X3}", curofs);
            bCursorI1.Text = string.Format("{0:F3} A", i1);
            bCursorI2.Text = string.Format("{0:F3} A", i2);
            bCursorVoltage.Text = string.Format("{0:F2} V", vcc);

            int dt = 50;
            double sx = 0, sy1 = 0, sxy1 = 0, sy2 = 0, sxy2 = 0, sx2 = 0, s1 = 0, svcc = 0;

            for (int i = curofs - dt; i < curofs + dt; i++)
            {
                if ((i & 0xFFF) != i)
                    continue;

                sx += i * time_scale;
                sx2 += i * i * time_scale * time_scale;
                sy1 += adc1[i];
                sxy1 += i * adc1[i] * time_scale;
                sy2 += adc2[i];
                sxy2 += i * adc2[i] * time_scale;
                s1 += 1;
                svcc += adc3[i]; 
            }

            double L1 = (svcc/s1) * (s1 * sxy1 - sx * sy1) / (s1 * sx2 - sx * sx);
            double L2 = (svcc/s1) * (s1 * sxy2 - sx * sy2) / (s1 * sx2 - sx * sx);

            bCursorL.Text = string.Format("{0:F0} uH", L1 * 1000);
            bCursorL2.Text = string.Format("{0:F0} uH", L2 * 1000);
        }
    }
}
