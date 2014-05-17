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
using System.IO;
//osa Y se musí změnit podle hodnot aby se tam vlezly
//Je potřeba metoda která vypočte závislost indukce na proudu
// zvetsit u portu pocet bytu nez se ma spustit read na 9 (navic 1 byte na cteni zesileni proudu)
// pridat do data recieved buffer na 1 byte, ktery bude brat ten byte od zesileni
// napred znormalizovat a pak jeste podelit zesilenim proudu
// meri se id, napeti, proud 1, proud 2, funguje na stridacku jen jeden proud, druhy tam ma nejaky srance

namespace tester
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();  // inicializuje prvky ve formu
        }

        private void chart1_Click(object sender, EventArgs e)
        {

        }

        UInt16 oldid = 0;
        UInt16 knee = 0;
        uint points_per_mes = 1000;
        uint number_of_mes = 3;
        uint id_of_mes = 0;
        bool curr_graph = true;

        double[] adc1 = new double[0x1000];     //pole pro hodnoty napeti   (volty)
        double[] adc2 = new double[0x1000];     //pole pro hodnoty proudu ve forme napeti   (volty)
        double[] adc3 = new double[0x1000];     //pole pro vypoctene hodnoty proudu     (ampery)
        double[] adc4 = new double[0x1000];     //pole pro vypoctene hodnoty indukcnosti    (henry)
        double[] adc5 = new double[0x1000];     //pole pro hodnoty zesileni proudu

        double adc1_offset = 237;               //pro napeti
        double adc2_offset = 100;             //pro prvni proud
        double adc3_offset = 100;              //pro druhy proud
        double adc5_offset = 0;              //pro zesileni

        double adc1_gain = 1000;                //napeti
        double adc2_gain = 1000;           //prvni proud
        double adc3_gain = 1000;           //druhy proud
        double adc5_gain = 1;           //zesileni

        double time_scale = 0.0238;             //rozliseni casove zakladny
        double resistance = 0.33;               //odpor pro vypocet proudu


        private UInt16 ReadU16(System.IO.Stream stm)
        {
            // přečte 2 byty ze streamu a vrátí je jako unsigned int16
            byte[] buffer = new byte[2];
            stm.Read(buffer, 0, 2);
            return (UInt16)(buffer[0] + buffer[1] * 256);
        }

        private void port_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            byte[] buffer = new byte[1];    //buffer pro zesileni
            double pomocnik = 0;

            while (port.BytesToRead > 9)
            {
                UInt16 id = ReadU16(port.BaseStream);

                if ((oldid + 1 != id) && (id != 1) && (oldid != 0xFFF)) //asi zjistuje jestli se neztratily zadny data
                {
                    Trace.WriteLine("LOST");
                    oldid = 0;
                    continue;
                }

                oldid = id;

                if (id * number_of_mes + id_of_mes <= 0xFFF)
                {
                    port.BaseStream.Read(buffer, 0, 1);
                    adc5[id * number_of_mes + id_of_mes] = ((UInt16)buffer[0] - adc5_offset) / adc5_gain;       //zesileni

                    adc1[id * number_of_mes + id_of_mes] = (ReadU16(port.BaseStream) - adc1_offset) / adc1_gain; //napeti

                    adc2[id * number_of_mes + id_of_mes] = (ReadU16(port.BaseStream) - adc2_offset) / adc2_gain / adc5[id * number_of_mes + id_of_mes];    //prvni proud
                    pomocnik = (ReadU16(port.BaseStream) - adc3_offset) / adc3_gain / adc5[id * number_of_mes + id_of_mes];    //druhy proud
                    if (adc2[id * number_of_mes + id_of_mes] < pomocnik)                                                        //porovnani proudu
                        adc2[id * number_of_mes + id_of_mes] = pomocnik;
                }
                else
                    port.DiscardInBuffer();
            }

            if (id_of_mes == (number_of_mes - 1))                     //spusti se az kdyz je dokonceno cele mereni
            {
                for (int i = 0; i < 0xFFF; i++)
                {
                    adc3[i] = adc2[i] / resistance;                     //vypocita proud
                    if (i > 1)
                        adc4[i - 2] = adc1[i - 1] * (adc3[i] - adc3[i - 2]) / (2 * time_scale);       //vypocita indukcnost
                }

                for (int i = 0; i < 0xFFF; i++)
                {
                    chart1.Series[1].Points.AddXY(adc3[i], 0);   //nastavi body Y na 0 a X na hodnoty podle hodnot proudu
                }

                id_of_mes = 0;
            }
            else
                id_of_mes++;
        }

        private void Form1_Load(object sender, EventArgs e) //tohle se zavola pri vytvoreni formu
        {
            chart1.Series[0].Points.Clear();  //vymaze body v grafu
            chart1.Series[1].Points.Clear();

            string[] ports = System.IO.Ports.SerialPort.GetPortNames();
            int n = 0;
            foreach (string single_port in ports)
            {
                ComboPortBox.Items.Add(single_port);
                n++;
            }

            TextBoxMesPoints.Text = points_per_mes.ToString();
            TextBoxMesRepeat.Text = number_of_mes.ToString();

            if (ports != null && ports.Length != 0)
            {
                this.ComboPortBox.SelectedItem = ports[0];
                //port.Open();
            }
        }

        private void button1_Click(object sender, EventArgs e) //vykresli graf proudu v zavislosti na case
        {
            double min = adc3[0];
            double max = adc3[0];

            curr_graph = true;

            chart1.Series[0].Points.Clear();
            chart1.Series[1].Points.Clear();

            for (int i = 0; i < 0xFFF; i++)
            {
                chart1.Series[0].Points.AddXY(i * time_scale, adc3[i]);      //vykresli ho
                if (adc3[i] < min)
                    min = adc3[i];
                if (adc3[i] > max)
                    max = adc3[i];
            }

            if (max > min)
            {
                chart1.ChartAreas[0].AxisY.Interval = (System.Math.Ceiling(max) - System.Math.Floor(min)) / 20;
                chart1.ChartAreas[0].AxisY.Minimum = System.Math.Floor(min);
                chart1.ChartAreas[0].AxisY.Maximum = System.Math.Ceiling(max);
                chart1.ChartAreas[0].AxisX.Maximum = System.Math.Ceiling(adc3.Count(i => i != 0) * time_scale);
                chart1.ChartAreas[0].AxisX.Interval = chart1.ChartAreas[0].AxisX.Maximum / 20;
                chart1.Refresh();
            }
        }

        private void button2_Click(object sender, EventArgs e) //vykresli graf indukcnosti v zavislosti na proudu
        {
            double min = adc4[0];                                   //promenne pro urceni minimalni a maximalni hodnoty na Y ose grafu
            double max = adc4[0];
            double max_proud = adc3[0];

            double last_angle = 0;
            double max_dif = 0;

            curr_graph = false;

            chart1.Series[0].Points.Clear();
            chart1.Series[1].Points.Clear();

            for (int i = 0; i < 0xFFD; i++)
            {
                chart1.Series[1].Points.AddXY(adc3[i+1], adc4[i]);      //vykresluje jednotlive body charakteristiky

                if (adc4[i] < min)                                  //porovnava min a max hodnoty s hodnotami ve vektoru indukcnosti a bere mensi/vetsi hodnotu
                    min = adc4[i];
                if (adc4[i] > max)
                    max = adc4[i];

                if (i == 0)                                                                                     //spocte prvni uhel
                    last_angle = Math.Atan((adc4[i + 1] - adc4[i]) / (adc3[i + 2] - adc3[i + 1]));
                else                                                                             //spocte druhy uhel, rozdil prvniho a druheho porovna s maximalnim rozdilem
                {                                                                                               //koleno charakteristiky je tam kde je maximalni rozdil mezi uhly spojnic bodu
                    double this_angle = Math.Atan((adc4[i + 1] - adc4[i]) / (adc3[i + 2] - adc3[i + 1]));
                    if ((this_angle - last_angle) > max_dif)
                    {
                        max_dif = this_angle - last_angle;
                        knee = (UInt16)i;                                                                       //ulozi pozici kolena charakteristiky
                    }
                }

                if (adc3[i+1] > max_proud)
                    max_proud = adc3[i+1];
            }

            if (max > min)
            {
                chart1.ChartAreas[0].AxisY.Interval = (System.Math.Ceiling(max) - System.Math.Floor(min)) / 20;         //nastavi interval mezi znackami na ose Y
                chart1.ChartAreas[0].AxisY.Minimum = System.Math.Floor(min);                                            //nastavi minimum a maximum osy Y
                chart1.ChartAreas[0].AxisY.Maximum = System.Math.Ceiling(max);
                chart1.ChartAreas[0].AxisX.Maximum = System.Math.Ceiling(max_proud);         //nastavi delku osy X
                chart1.ChartAreas[0].AxisX.Interval = chart1.ChartAreas[0].AxisX.Maximum / 20;                          //nastavi interval mezi znackami na ose X
                chart1.Refresh();
            }

            LabelKolenoI.Text = string.Format("{0:F2} mA", adc3[knee+1] * 1000);
            LabelKolenoL.Text = string.Format("{0:F0} uH", adc4[knee] * 1000 * 1000);
        }


        private void numericUpDown1_ValueChanged(object sender, EventArgs e)
        {
            chart1.ChartAreas[0].CursorY.Position = (float)numericUpDown1.Value;
        }

        private void chart1_MouseMove(object sender, MouseEventArgs e)
        {
            Point mousePoint = new Point(e.X, e.Y);

            chart1.ChartAreas[0].CursorX.SetCursorPixelPosition(mousePoint, true);

            double curpos = 0;
            int curofs = 0;
            double volt = 0;
            double curr = 0;
            double induct = 0;

            if (curr_graph == true)
            {
                curpos = chart1.ChartAreas[0].CursorX.Position;
                curofs = (int)(curpos / time_scale);
                volt = adc1[curofs & 0xFFF];
                curr = adc3[curofs & 0xFFF];
                if (curofs > 0)
                    induct = adc4[(curofs - 1) & 0xFFF];
            }
            else
            {
                curr = chart1.ChartAreas[0].CursorX.Position;
                curofs = (int)Array.IndexOf(adc3, curr);
                volt = adc1[curofs & 0xFFF];
                if (curofs > 0)
                    induct = adc4[(curofs - 1) & 0xFFF];
                curpos = curofs * time_scale;
            }

            bCursorTime.Text = string.Format("{0:F2} us", curpos / 100);
            bCursorSample.Text = string.Format("0x{0:X3}", curofs);
            bCursorI1.Text = string.Format("{0:F3} V", volt);
            bCursorI2.Text = string.Format("{0:F3} A", curr);
            bCursorVoltage.Text = string.Format("{0:F2} H", induct);

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
        }

        private void PortSelected(object sender, EventArgs e)
        {
            port.Close();
            string selected = this.ComboPortBox.GetItemText(this.ComboPortBox.SelectedItem);
            port.PortName = selected;
           // port.Open();
        }

        private void NumberOfMesChanged(object sender, EventArgs e)
        {
            if (points_per_mes * Convert.ToUInt16(TextBoxMesRepeat.Text) > 0xFFE)
            {
                number_of_mes = (uint)Math.Floor(0xFFF / (double)points_per_mes);
                TextBoxMesRepeat.Text = number_of_mes.ToString();
            }
            else
                number_of_mes = Convert.ToUInt16(TextBoxMesRepeat.Text);
        }

        private void NumberOfSamplesChanged(object sender, EventArgs e)
        {
            if (number_of_mes * Convert.ToUInt32(TextBoxMesPoints.Text) > 0xFFE)
            {
                points_per_mes = (uint)Math.Floor(0xFFF / (double)number_of_mes);
                TextBoxMesPoints.Text = points_per_mes.ToString();
            }
            else
                points_per_mes = Convert.ToUInt32(TextBoxMesPoints.Text);
        }

        private void Button3_Click(object sender, EventArgs e)
        {
            id_of_mes = 0;
        }
    }
}
