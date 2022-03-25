#include <sstream>

#include "logger-passport.h"

/**
 * Initialize logger password directory
 * @param passwords_path path to the catalog with password files
 * @param verbosity if 1, 2 or 3 print out to the stderr errors parsing declarations
 * @return descriptor of the passwords to be passed to the parsePacketsToSQLClause()
 */
void* initLoggerPasswords(
        const std::string &passwords_path,
        const int verbosity // default 0
)
{
    return NULL;
}

/**
 * Destroy and free logger properties
 * @param env descriptor
 */
void doneLoggerPasswords(void *env)
{
}

const char *DS2438_CODE = "26";
const char *DS18B20_CODE = "28";
const char *MMA_CODE = "00";
const char *DS2411_CODE = "01";
const char *DS2450_CODE = "20";

Single Koeff::compute(Single x) // вычисл. реальной темп. с учетом коэф. паспорта
{
    return (a * (x * x * x * x * x) + b * (x * x * x * x) + c * (x * x * x) + d * (x * x) + e * x + f);
}

bool Koeff::check(Single x)     //проверка попадания в разрешенный диапазон
{
    if ((x < eTemp) && (x > sTemp)) { return true; }
    else { return false; }
}

Koeff::Koeff(std::string nmac, Single na, Single nb, Single nc, Single nd, Single ne, Single nf, Single nst, Single nen,
             std::string nkosa, int nIndex)
{
    mac = nmac;    //
    a = na;    // 0
    b = nb;    // 0
    c = nc;    // 0
    d = nd;    // 0
    e = ne;    // a=1
    f = nf;    // b=0
    sTemp = nst;    // -274
    eTemp = nen;    // +500
    kosa = nkosa;    //
    index = nIndex;     // индекс в паспорте косы - для угла по 2 индекса!!
}

Koeff::Koeff()   //констр пустой
{
    mac = "";    //
    a = 0;    // 0
    b = 0;    // 0
    c = 0;    // 0
    d = 0;    // 0
    e = 0;    // a=1
    f = 0;    // b=0
    sTemp = -274;    // -274
    eTemp = +500;    // +500
    kosa = "";    //
    index = 0;     // индекс в паспорте косы - для угла по 2 индекса!!
}

KoeffXYZ::KoeffXYZ(std::string nmac, Single x0off, Single y0off, Single z0off, Single xs, Single ys, Single zs,
                   std::string nkosa, int nIndex)
{
    mac = nmac;   // начало с 00
    kx0off = x0off;    // 0 offset плавающее
    ky0off = y0off;    // 0
    kz0off = z0off;    // 0
    kxs = xs;    // 0.02197265625 коэф.
    kys = ys;    // 0.02197265625
    kzs = zs;    // 0.02197265625
    kosa = nkosa;    //
    index = nIndex;     // индекс в паспорте косы
}

/*
// вычисление отрацат.темп. для ds2438
static Single prog::minusTempCompute(int msb, int lsb)
{
    Single t;
    //Int16 tmp;
    int tmp;
    int mtemp = Convert.ToInt32(Convert.ToByte(msb) * 0x100 + Convert.ToByte(lsb & 0xF8));
    mtemp = (mtemp >> 3);
    //mtemp = Convert.ToInt16(mtemp | 0xE000);
    mtemp = (mtemp | 0xE000);
    mtemp = ~mtemp;
    //tmp = Convert.ToInt16(mtemp + 1);
    tmp = (mtemp + 1);
    tmp = tmp & 0x0000FFFF;
    t = -Convert.ToSingle(tmp) / 32;
    //t = -Convert.ToSingle(~(Convert.ToByte(msb) * 0x100 + Convert.ToByte(lsb & 0xF8)) + 1) / 256;
    return t;
}
/*

/*
    Angl::Angl()
        {
            //this->num_zamer = 0;
            //this->num_poll = 0;
            this->mac = "";
            this->index = -1;
            this->kosa = "NONE";
            this->temp = -999;
            this->time = "";
            this->versionK485 = "1.00";  //
            this->ID_Slave = 0x01;       //
            this->count_X = this->count_Y = this->count_Z = 0;
            this->anglX = 0;       //
            this->anglY = 0;       //
            this->anglZ = 0;       //

            this->quadrant = 0;       //
            this->shift = 0;       //
            this->shift_Y = 0;       //
            this->shift_Z = 0;       //
        };
    
        // mac не добавлен?! мас идет из листа ИД в порядке ИД!! а не паспорта
    void Angl::tempAdd(int lsb, int msb, std::string ds)   //сырая темп.
        {
            Single t;
            if (ds == "DS2438")
            {
                //13 bit, over the range of -55C to +125C in 0.03125C increments.
                //2-1 2-2 2-3 2-4 2-5 0 0 0 LSB
                //MSb (unit = °C) LSb
                //S 26 25 24 23 22 21 20 MSB    макс 0x7F = 127 grad
                //
                if (Convert.ToByte(msb) < 0x80) //+
                {
                    t = Convert.ToSingle((Convert.ToByte(msb) % 0x80) * 0x100 + Convert.ToByte(lsb)) / 256;
                }
                else  //- -0,5 = FF80h, ff/0x80=1 lsb=0x80 0x0180 ( 0x1000 -)= E80 3712 ( 0x2000 -)= 1E80 7808
                {
                    //t = -Convert.ToSingle(0x1000 - ((Convert.ToByte(msb) % 0x80) * 0x100 + Convert.ToByte(lsb))) / 256;
                    //t = -Convert.ToSingle(~(Convert.ToByte(msb) * 0x100 + Convert.ToByte(lsb & 0xF8)) + 1) / 256;
                    t = prog.minusTempCompute(msb, lsb);
                }
            }
                //12 bit
                //BIT 7 BIT 6 BIT 5 BIT 4 BIT 3 BIT 2 BIT 1 BIT 0
                //LS BYTE 2**3 2**2 2**1 2**0 2-1 2-2 2-3 2-4
                //BIT 15 BIT 14 BIT 13 BIT 12 BIT 11 BIT 10 BIT 9 BIT 8
                //MS BYTE S S S S S 26 25 24
            else //ds18b20     0x1000 =  4096  2^7=128 2^11=2048
            {
                if (Convert.ToByte(msb) < 0xF0)
                {
                    t = Convert.ToSingle((Convert.ToByte(msb) % 0x10) * 0x100 + Convert.ToByte(lsb)) / 16;
                }
                else
                {
                    t = -Convert.ToSingle(0x1000 - ((Convert.ToByte(msb) % 0x10) * 0x100 + Convert.ToByte(lsb))) / 16;
                }
            }
            this->temp = t;
        }

        // класса Angl - заполнение счета в двойной со знаком  // ref values[i];
    void Angl::countsAdd(ref uint16_t valueX, ref uint16_t valueY, ref uint16_t valueZ)  //= values[i];
        {
            uint16_t countX;     // приходят msb lsb  ?
            uint16_t countY;
            uint16_t countZ;

            if (valueX > 0x7F00) //minus            // как в выводе на экран !!!
            {
                countX = (uint16_t)(~valueX + 1); //data.Word = ~data.Word + 1;
                countX = (uint16_t)(countX >> 2);
                this->count_X = -(Convert.ToDouble(countX));    //
                this->znak_X = true;
            }
            else { countX = (uint16_t)(valueX >> 2); this->count_X = Convert.ToDouble(countX); } // plus

            if (valueY > 0x7F00) //minus
            {
                countY = (uint16_t)(~valueY + 1); //data.Word = ~data.Word + 1;
                countY = (uint16_t)(countY >> 2);  //
                this->count_Y = -(Convert.ToDouble(countY));    //
                this->znak_Y = true;
            }
            else { countY = (uint16_t)(valueY >> 2); this->count_Y = Convert.ToDouble(countY); } // plus

            if (valueZ > 0x7F00) //minus
            {
                countZ = (uint16_t)(~valueZ + 1); //data.Word = ~data.Word + 1;
                countZ = (uint16_t)(countZ >> 2);          //
                this->count_Z = -(Convert.ToDouble(countZ));    //
                this->znak_Z = true;
            }
            else { countZ = (uint16_t)(valueZ >> 2); this->count_Z = Convert.ToDouble(countZ); } // plus
        }

        //функции для  Angl !!! расчет gr cosX не по среднему углу, а по средним count !!!!!
    void Angl::setAngl()                       //public Angl set0(Angl a, Angl0 a0)
        {
            this->len_vec = Math.Sqrt(this->count_Y * this->count_Y + this->count_Z * this->count_Z + this->count_X * this->count_X); //общая менее 1 !!!
            this->yz_kv = this->count_Y * this->count_Y + this->count_Z * this->count_Z;     //
            this->yz = Math.Sqrt(this->yz_kv);        // часть только всегда + !!
            this->shift = this->yz / this->len_vec;    // часть от общей!?? только + !!
            // НАДО РАСЧЕТ ЧАСТИЧНЫХ СДВИГОВ!!! по Y Z
            //      if (this->znak_X == true) this->count_X = -this->count_X;     // как Y ??? если угол - счет Х всегда + до переворота!!?
            //      if (this->znak_Y == true) this->count_Y = -this->count_Y;     //2 или 3 квадр. если угол - this->yz  this->shift  !!!!!!!!
            //      if (this->znak_Z == true) this->count_Z = -this->count_Z;     //2 или 1 если угол -

            this->shift_Y = count_Y / this->len_vec;  //count_Y shift_Y со знаком!!?? часть от общей!?? по Y
            this->shift_Z = count_Z / this->len_vec;  //count_Z shift_Z со знаком!! часть от общей!?? по Z
            // надо умножать на txtDist.Text в мм это 1000

            this->gr = this->len_vec / Convert.ToDouble(4096);  //4096 max

            this->anglX = Math.Atan(yz / this->count_X) * 180 / Math.PI;  // угол Х - при вертикали 4096 или 1g около 0 всегда в + до 90 град??

            // угол влево y в плюс, угол от себя z в плюс, rad проекции углов !!!! на оси Y Z
            // проверить квадранты!!! минус на минус даст + ?????
            // нормировать на ????

            this->anglY = Math.Atan(this->count_Y / this->count_X) * 180 / Math.PI;  // grad  Y относит Х
            this->anglZ = Math.Atan(this->count_Z / this->count_X) * 180 / Math.PI;  // grad  Z относит Х

            //нормальые углы по косинусу - от знаков X Y Z минус на минус деление дает + !!!
            // косинус - рад дает + угла,
            //rad  -0,02	1,59079766	угол аcos, -0,019997334	угол аtan

            //ПРОВЕРИТЬ!!! знаки!! косинус XYZ всегда +???
            this->cosanglX = Math.Acos(this->count_X / this->len_vec) * 180.0 / Math.PI;  // grad по cosX = град по тангенс !!!!
            this->cosanglY = Math.Acos(this->count_Y / this->len_vec) * 180.0 / Math.PI;  // grad
            this->cosanglZ = Math.Acos(this->count_Z / this->len_vec) * 180.0 / Math.PI;
            this->gg = this->cosanglX * this->cosanglX + this->cosanglY * this->cosanglY + this->cosanglZ * this->cosanglZ;
            if (this->count_Y < 0)    // minus
            {
                this->anglX = -this->anglX;    // minus 2 3 квадрант относит Y те лево право, Х тоже с минусом!? во 2 и 3 квадр.??
                this->shift = -this->shift;  //общее смещение считаем минусом как и угол
                if (this->count_Z < 0) { this->quadrant = 3; }  // minus minus 3 квадрант// minus
                else { this->quadrant = 2; }        // minus plus
            }
            else                //plus
            {
                if (this->count_Z < 0) { this->quadrant = 4; } // plus minus 3 квадрант   // minus
                else { this->quadrant = 1; }     // plus plus
            }
        }

        // сохранять для калибровки!! valueX ! в доп коде
        // COUNT_GRAD X Y Z может быть разным - в паспорте?? разная чувств.? по тангенсу!
    void Angl::anglAdd(uint16_t valueX, uint16_t valueY, uint16_t valueZ)  //= values[i];
        {
            this->valX = valueX;     // сохранить вывод ММА
            this->valY = valueY;
            this->valZ = valueZ;

            //заполняет count_X znak_X
            countsAdd(ref this->valX, ref this->valY, ref this->valZ);

            //все вычисления
            setAngl();


    double Angl::computeAngl(uint16_t value)
        {
            bool znak = false;
            //Int16 count;
            if (value > 0x7F00) //minus для Х
            {
                value = (uint16_t)(~value + 1);   //data.Word = ~data.Word + 1;
                value = (uint16_t)(value >> 2);   //14 bit count без знака
                //count = - (Int16)value;
                znak = true;
            }
            else
            {
                // count = (Int16)value;
                value = (uint16_t)(value >> 2); // plus      mg/count 0,244140625
            }
            //1 count = 0.25 mg, 1g = 90 grad = 4096 .   0.02197265625 grad
            //double anglxyz = value * koeff;  //COUNT_GRADX; //
            double anglxyz = value * 0.02197265625; //COUNT_GRAD;  // 0.02197265625;
            if (znak) anglxyz = -anglxyz; //
            return anglxyz;
        }
        // проверять что коса найдена!! и в ней все датчики - ищет по МАС, назначает номер косы
        // ошибка одна - МАС не найден в паспорте!
        // проверка мас по ВСЕМУ паспорту и заполнение мас и индекса
    bool Angl::CheckMac(std::vector<Koeff> koef_data)
        {
            this->kosa = "NONE";             //ID_Slave?
            for (int i = 0; i < koef_data.Count; i++)
            {
                if (this->mac == koef_data[i].mac)   //мас найден в паспорте     косе
                {
                    this->kosa = koef_data[i].kosa;      // назначение косы
                    this->index = koef_data[i].index;    // индекс в паспорте     косе
                    return true;
                }
            }
            //("MAC не найден в файле паспорта. MAC = {0:S}", this->mac);
            return false;
        }
        //
    bool Angl::computeTemp(std::vector<Koeff> koef_data)  //std - c 2 коэф. на все, poly - или с 9
        {
            //this->err = "begin";
            kosa = "NONE";
            for (int i = 0; i < koef_data.Count; i++)
            {
                if (this->mac == koef_data[i].mac)        //мас в паспорте
                {
                    //this->err = "mac";
                    this->kosa = koef_data[i].kosa;          // коса в паспорте
                    this->index = koef_data[i].index;         // индекс в паспорте по умолч. =-1
                    if (koef_data[i].check(this->temp) == true)  // проверка диапазона температур для коэф.
                    {
                        //this->err = "check";
                        this->newTemp = koef_data[i].compute(this->temp);  // вычисление newTemp с коэфф.!!
                        return true;
                    }
                    else        // темп. не в диапазоне - пропуск - ищем дальше, еще коэф?
                    {
                        //this->err = "not check";
                        //("Температура НЕ в диапазоне!. Kosa = {0:S}, MAC = {1:S}", this->kosa, this->mac);
                    }
                }

            }
            //МАС не в паспорте
            //Console.WriteLine("MAC не найден в файле паспорта. MAC = {0:S}", this->mac);
            return false;
        }
    bool Angl::computeTempPol(std::vector<Koeff> koef_data)   // c 3 коэф. на 3 диапазона
        {
            kosa = "NONE";
            //int nom_koef = 0;
            for (int i = 0; i < koef_data.Count; i = i + 3) //шаг 3 - по 3 строки на датчик
                //for (int i = 0; i < koef_data.Count; i++)   //i  их в 3 раза больше чем МАС
            {
                if (this->mac == koef_data[i].mac)
                {
                    //nom_koef = nom_koef + 1;
                    for (int j = 0; j < 3; j++)
                    {
                        if (koef_data[i + j].check(this->temp) == true)
                        {
                            this->kosa = koef_data[i + j].kosa;
                            this->index = koef_data[i + j].index;
                            this->newTemp = koef_data[i + j].compute(this->temp);
                            return true;
                        }
                    }

                }
            }
            //МАС не в паспорте
            //Console.WriteLine("MAC не найден в файле паспорта. MAC = {0:S}", this->mac);
            return false;
        }
    void Angl::pwrAdd(int lsb) //B3 = 179
        {
            int vbat = (Int16)(0x00FF & lsb); //76 1.52v
            if (vbat < 0x50)
            {
                this->vBat = Convert.ToSingle(4.4 - (vbat * 0.022));  // 0т 0 растет при напр.менее 4.4в переход 0xFF -> 0x00
            }
                //this->vBat = Convert.ToSingle(4.4 + (255 - (vbat + 46)) * 0.020);   //4.4 + 2.706 = 7.106v те 2.1в лишние
                // сейчас дает 0х23 =35
            else
            {
                this->vBat = Convert.ToSingle(4.4 + ((255 - vbat + 10) * 0.022));
            }
        }
        //добавить замер, лист коэф. koeff_list_angl для температуры! только?
    bool Angl::recValAdd(uint16_t val[], KoeffType type, std::vector<Koeff> koef_data)  //short добавить int type !!
        {
            bool ret = true;        //темп. не в диапазоне or МАС не найден в паспорте
            //добавляет temp
            tempAdd(val[4] & 0x00FF, (val[4] & 0xFF00) >> 8, "DS2438");  //values[4] & 0x00FF values[4] & 0xFF00
            //учет коэфф.
            if (type == KoeffType.Poly)    // по полиному!!!
            {
                if (!computeTempPol(koef_data)) { ret = false; }   //
            }
            else if (type == KoeffType.Std)          //newTemp темп не в диап.
            {
                if (!computeTemp(koef_data)) { ret = false; } //МАС не найден в паспорте
            }
            else if (type == KoeffType.Angl)   //для углов (новых!) Angl файл angl_pas.txt
            {
                if (!computeTemp(koef_data)) { ret = false; } //МАС не найден в паспорте
            }
            //KoeffXYZ - нет в перечислении, есть лист koeff_list_angl

            //добавить углы xyz в градусах тоже
            anglAdd((uint16_t)val[1], (uint16_t)val[2], (uint16_t)val[3]); //values[]
            pwrAdd(val[0] & 0x00FF);       //values[0] & 0x00FF напряжение батареи
            return ret;
        }
    }
*/

/*
    // класс паспортов
    public static class passport
    {
    public  static uint8_t[] getStringByte(string filename, int mode)  //static
        {
            std::vector<uint8_t> strbyte = new std::vector<uint8_t>(); //

            if (mode == 0)
            {
                using (StreamReader sr = File.OpenText(filename))   // закроет!! UTF8 !!! но умолчанию!!
                {
                    //sr.ReadToEnd          // читать до конца
                    //sr.CurrentEncoding // кодировка
                    //try
                    ///{
                    sr.BaseStream.Position = 0;  // начать с 0
                    if (sr.EndOfStream == false) //позиция в конце потока - иначе читать символ и sr.BaseStream.Position++;
                    {
                        strbyte.Add((uint8_t)sr.Read()); // чтение следующий символ инт!!!!
                    }
                    return strbyte.ToArray();
                }
            }

            //try
            ///{
            using (Stream fs = new FileStream(filename, FileMode.Open, FileAccess.Read))
            {
                //какая длина!!
                long lenfile = fs.Length;  //длина файла??

                if (mode == 4)
                    return StreamExtensions.ReadAllBytes(fs);

                //Вы можете использовать эту функцию с пользовательскими классами. Вам просто нужно добавить [Serializable]
                //атрибут в свой класс, чтобы включить сериализацию

                //uint8_t[] bytes = BitConverter.GetBytes(value);
                //uint8_t[] byteArray = BitConverter.GetBytes(argument); // char

                //File.Open(filename, FileMode.Open, FileAccess.Read))  //Text
                //FileStream sr = new File.Open(filename)

                if (mode == 1)
                {
                    using (BinaryReader br = new BinaryReader(fs, Encoding.ASCII))  //    fs.Close();
                    {
                        long lenread = br.BaseStream.Length;  //длина файла??
                        // while (sr     // StreamReader     .EndOfStream
                        // while (sr.Peek() >= 0) //.EndOfStream
                        while (br.PeekChar() >= 0)   //Peek  error -1
                        {
                            //uint8_t br = sr.Read();
                            strbyte.Add(br.ReadByte()); //
                        }
                        //uint8_t[] rbytes new uint8_t[strbyte.Count];  //error
                        return strbyte.ToArray();
                    } //BinaryReader br
                }
                //
                if (mode == 2)
                {
                    BinaryFormatter bf = new BinaryFormatter();
                    using (var ms = new MemoryStream())
                    {
                        fs.CopyTo(ms);          //все в память - длина??
                        bf.Serialize(ms, fs);  //obj
                        return ms.ToArray();
                    }
                }
                if (mode == 3) //
                    return ObjectToByteArray(fs);     //Object obj
            }  //Stream fs
            return strbyte.ToArray(); // пустой!!!!!
        }

        //НЕ ИСПОЛЬЗОВАНО!?? Convert an object to a uint8_t array - и поток тоже ???
    public static uint8_t[] ObjectToByteArray(Object obj)
        {
            BinaryFormatter bf = new BinaryFormatter();
            using (var ms = new MemoryStream())
            {
                bf.Serialize(ms, obj);
                return ms.ToArray();
            }
        }
        //НЕ ИСПОЛЬЗОВАНО!?? Convert a uint8_t array to an Object
    public static Object ByteArrayToObject(uint8_t[] arrBytes)
        {
            using (var memStream = new MemoryStream())
            {
                var binForm = new BinaryFormatter();
                memStream.Write(arrBytes, 0, arrBytes.Length);
                memStream.Seek(0, SeekOrigin.Begin);
                var obj = binForm.Deserialize(memStream);
                return obj;
            }
        }

        //добавить счетчики в класс и заполнять
        // private int all_MAC;  // всего MAC
        // private int passport_MAC;  //
        // private int polynom_MAC;  //

        //функции для счетчиков внутренних!??

    public static int razbor(string str, ref std::vector<string> sub_list)
        {
            //std::vector<string> sublist = new std::vector<string>();
            string sub_str = "";
            int n = 0;

            //1 :28FF30DD821502D2:-30,25   :-10,25  :-0,25694:9,75  :22,4375:23,0625:29,79861
            //LOG.log_str("razbor: " + str);  //
            str.Trim();  //начальные и конечные пробелы удалять ! ТАБ не удаляет!
            if (str.Length == 0) { return 0; } //Возрашаем -были только пробелы!

            for (int i = 0; i < str.Length; i++)      //кончается 0х00 ! и в sub_str может быть подстрока!
            {
                //0x09=ТАБ
                if ((str[i] == ' ') || (str[i] == ':') || (str[i] == 0x09) || (str[i] == '.') || (str[i] == ';'))     //далее
                {
                    //LOG.log_str("razbor:sub_str:" + sub_str);  //
                    if (sub_str == "")       //еще нет в подстроке ничего, два проб.или :: подряд не считать
                    {
                        //LOG.log_str("razbor: Пропуск разделителя");  //
                        continue;
                    }
                    //между разделителями что то должно быть! или просто пропустит
                    sub_list.Add(sub_str);                  // сохранить подстроку
                    //LOG.log_str("razbor: n=" + n + " sub=" + sub_str);  //
                    sub_str = "";
                    n++;                            // с 1
                    continue;
                }
                sub_str = sub_str + str[i];        // добавить символ
            }
            if (sub_str != "")        // не пустая
            {
                sub_list.Add(sub_str);
                //LOG.log_str("razbor: n=" + n + " sub=" + sub_str);  //
                n++;       //
            }
            return n;  // кол. подстрок
        }

    public static string unComment(string str)  //static
        {
            string tmp = "";
            string out_str = "";
            bool space = false;

            if (str == null) { return null; }      //Возрашаем пусто - это конец файла!??
            if (str.Length == 0) { return "###"; } //Возрашаем решетки
            if (str[0] == '#') { return "###"; }   //Возрашаем решетки//Если это строка коментарий
            //Иначе вытаскиваем полезную составляющую до коментария
            for (int i = 0; i < str.Length; i++)   //кончается 0х00 ?
            {
                if (str[i] == '#') { break; }      //далее коментаарии - выход
                //все ТАБ заменить пробелом! ( или вообще все символы от 0х01 до 0x1F  ???)
                if (str[i] == 0x09)         //tab горизонтальный замена пробелом
                {
                    //if (Form1.fdbg == true) LOG.log_str(String.Format("Замена ТАБ: 0x{0:X2}", str[i] + 0x30)); // 0x39 = 9
                    tmp = tmp + ' ';
                }
                else { tmp = tmp + str[i]; }
            }
            // удалить пробелы более 1 в середине строки
            for (int i = 0; i < tmp.Length; i++)
            {
                if (tmp[i] != ' ')      // не пробелы - переписываем
                {
                    out_str = out_str + tmp[i]; //сохранить символ
                    space = false;      //сбросить признак что предыдущий тоже пробел
                }
                else            // пробел
                {
                    if (space == true) { continue; } // пробел не первый - пропуск символа
                    //иначе первый пробел
                    out_str = out_str + tmp[i];      //сохранить 1-й пробел
                    space = true;     //поставить признак что предыдущий тоже пробел
                    //continue;  // продолжить
                }
            }
            //            if (Form1.fdbg == true) LOG.log_str("Конец строки");
            return out_str.Trim();  //начальные и конечные пробелы удалять ! ТАБ не удаляет!
            //return tmp.Trim();      //начальные иконечные пробелы удалять ! ТАБ не удаляет!
        }

        //статик getString()  для байт массива?? или потока Encoding.UTF8 !!
        //  std::vector string ?? из хэш файла как текст - и запрашивать индекс строки ? 0 – пароль на вход
        //1 хэш паспорта, 2 полинома и тд

        //fileStream.CopyTo(memoryStream); //(this Stream instream)
        //поток уже есть!!!!
    public static std::vector<string> getString(Stream instream) //из любого потока без this!! это статик
        {
            std::vector<string> strlist = new std::vector<string>();
            //        uint8_t[] ba = StreamExtensions.ReadAllBytes(instream); // читать из файла в массив
            //text StringBuilder()   //Convert.ToChar(ctr)
            //  MemoryStream ms = new MemoryStream();

            //чтение как строки минимум есть 0з0D ВК -
            using (StreamReader sr = new StreamReader(instream, encoding: Encoding.UTF8))  //File.OpenText(filename)
            {
                string s = "";
                //далее все тоже!!!
                //while (sr.Peek() >= 0)
                while ((s = unComment(sr.ReadLine())) != null)  //EOF
                {
                    //s = unComment(sr.ReadLine());         //после удаления коментов и пробелов и таб может быть пуста!!
                    //   LOG.log_str("getString: Строка " + s); //
                    if (s.Length == 0) { continue; }  //если пустая - не писать - продолжить

                    //если строка начинается с коментов - делает unComment
                    if (s[0] != '#') { strlist.Add(s); }  //не коменты   было исключение Индекс находился вне границ массива.
                    //  strlist.Add(s) - добавить строку - даже пустую??
                    if (s == "NEND") { break; } // конец файла - если указан - не пишем !!
                }  //while
            }
            return strlist;
        }

    public static std::vector<string> getString(string filename)  //static
        {
            std::vector<string> strlist = new std::vector<string>();  //создан - уже не null !! ???
            if (!File.Exists(filename))
            {
                LOG.log_str("getString: Файл НЕ найден! Файл: " + filename);  //
                return strlist;                // будет с 0 длиной!!
            }
            //if (filename.Length) == "") { return strlist; } // пуст!
            // не нужно вызывать метод StreamReader.Close(), он будет вызван автоматически, при выходе их блока using
            try
            {
                //файлы паспортов могут быть в других кодировках?? windows-1251, CP1251, CP866, KOI8-R
                //сохранять как UTF8!!

                using (StreamReader sr = new StreamReader(filename, Encoding.UTF8)) //любой
                //кодировка неизвестна? как уникод попробует!!
                // using (StreamReader sr = File.OpenText(filename))  //только существующий!
                {
                    string s = "";

                    //.ReadToEnd() Читает все содержимое файла и возвращает результат в виде строки.
                    // int Peek(): возвращает следующий доступный символ, если символов больше нет, то возвращает -1
                    //но не перемещает текущую позицию на него. ( не использует его.)
                    //ReadLine() Читает строку из файла. Возвращает прочитанную строку.
                    //Возвращаемая строка не содержит завершающий возврат каретки или перевод строки!!!

                    //while (sr.Peek() >= 0)
                    while ((s = unComment(sr.ReadLine())) != null)  //EOF
                    {
                        //s = unComment(sr.ReadLine());         //после удаления коментов и пробелов и таб может быть пуста!!
                        //   LOG.log_str("getString: Строка " + s); //
                        if (s.Length == 0) { continue; }  //если пустая - не писать - продолжить

                        if (s[0] != '#') { strlist.Add(s); }  //не коменты   было исключение Индекс находился вне границ массива.
                        //  strlist.Add(s) - добавить строку - даже пустую??
                        if (s == "NEND") { break; } // конец файла - если указан - не пишем !!
                    }  //while
                } //using
            }  // try
            catch (Exception e)
            {
                LOG.log_str("getString: " + String.Format("The process StreamReader failed: {0}", e.ToString()));
                strlist.Clear();
                return strlist;     //std::vector<string> или вернуть что есть??
            }
            return strlist;  //вернуть что есть??
        }

//============================= LORA ====
        // для лоры добавить счетчики в класс и заполнять
        //   private int all_year;  // всего лет
    private static int cyr_year;  // текущий год.
    private static int num_cyr_year;  // кол.в тек.году

        //параметры устройства
    public struct dev_param
        {
        public int num;                 //номер в году НЕ ПИШЕТ пока
        public String devAddr;          //
        public String devAppSKey;       //
        public String devNwkSKey;       //

        public String devEUI;           //
        public String devAppKey;        //

        public String devHash;         //
        public String devAESKey;
        public string devDescr;         //описание
        }

        //структура для года и парам.
    public struct dev_year
        {
        public string Year;         //год
        public int n_year;         //
        public std::vector<dev_param> param;     //devaddr и парам для него
        public string Descr;         //
        }

        //НЕ ИСПОЛЬЗУЕТСЯ создать и заполнить структуру для года и параметров
    public  static void getEmptyStruct(int cyr_year)
        {
            // создать новый если новый год только !! пустой
            dev_year tmp_year = new dev_year(); //list нужен param
            //заполнить год
            tmp_year.Year = cyr_year.ToString();
            tmp_year.n_year = cyr_year;
            // создать пустой лист устройств
            dev_param tmp_par = new dev_param(); //
            tmp_par.devAddr = "";           // все пустые сделать

            // std::vector<dev_param> param;
            tmp_year.param.Add(tmp_par);   //

            // добавить в лист
            dev_param_list.Add(tmp_year);    //
        }

    public static int getNumCyrYear()
        {
            return num_cyr_year;
        }
    public static int getCyrYear()
        {
            return cyr_year;
        }
        //
    public static std::vector<dev_year> dev_param_list = new std::vector<dev_year> ();  //обьявить! лист устройств по годам
        //std::vector<dev_year> dev_param_list = new std::vector<dev_year>();

        //
        //структура - новые параметры для нового устройства - временные
        // private dev_param tmp_dev = new dev_param();

    public static bool loadDev(Stream mstream)
        {
            std::vector<string> str = getString(mstream);
            return loadDevStr(str);
        }

    public static bool loadDevStr(std::vector<string> str)
        {
            dev_param_list.Clear();            //
            int cur_str = 0;              // кол. строк в файле без коментов и лишних пробелов
            int cur_index_year = 0;             // кол лет - начало с N !!
            int cur_index_dev = 0;

            cur_str = str.Count; //строк в листе
            LOG.log_str("loadDev: Количество строк: " + str.Count); //

            for (int i = 0; i < str.Count; i++)  // по кол.строк в листе
            {
                if (str[i][0] == 'N')  //года
                {
                    int len = str[i].Length;
                    LOG.log_str("loadDev: длина str=" + len); // 3
                    dev_year tmp = new dev_year();
                    // tmp.Year = str[i];                          // будет отд с именем Nxх
                    string y = str[i].Substring(1, (len - 1));    //3  без N  ="20"
                    if (y.Length == 0)     // нет года !!!!!
                    {
                        LOG.log_str("loadDev: Нет года после N"); //
                        return false;
                    }
                    LOG.log_str("loadDev: длина года после N " + y.Length + ", y=" + y); //
                    tmp.Year = y;                           // стринг
                    tmp.n_year = Convert.ToInt32(tmp.Year, 10); // как хх
                    tmp.param = new std::vector<dev_param>();
                    dev_param_list.Add(tmp);                  // добавление номера года в dev_param_list

                    cur_index_year++;                 //
                    //     this->cyr_year = cur_index_year; //int - на выходе последний номер!
                    cyr_year = tmp.n_year;     // последний год
                    cur_index_dev = 0;
                    LOG.log_str("loadDev: N=" + tmp.Year + ", int=" + tmp.n_year.ToString()); //
                    num_cyr_year = 0;    // последний добавленный по году!!
                }
                else  // отдельные приборы - без проверок
                {
                    Regex reg = new Regex(@"([0-9a-fA-F]+)\s+([0-9\,\.\-]+)\s+([0-9\,\.\-]+)"); // убрать , и - !!!
                    Match m = reg.Match(str[i]);
                    //int dev_adr = m.Groups[1].Value;  // номер если

                    string dev_adr = m.Groups[1].Value;

                    //      NumberFormatInfo nfi = new CultureInfo("en-US", false).NumberFormat;
                    //      nfi.NumberDecimalSeparator = ",";
                    //      nfi.NegativeSign = "-";

                    string dev_appSKey = m.Groups[2].Value;
                    string dev_nwkSKey = m.Groups[3].Value;
                    //.......devEUI devAppKey devHash
                    string dev_EUI = m.Groups[4].Value;
                    string dev_App_Key = m.Groups[5].Value;
                    string dev_Hash = m.Groups[6].Value;        //
                    string dev_AESKey = m.Groups[7].Value;

                    dev_param dev_tmp = new dev_param();
                    dev_tmp.devAddr = dev_adr;
                    //......
                    dev_tmp.devAppSKey = dev_appSKey;
                    dev_tmp.devNwkSKey = dev_nwkSKey;
                    dev_tmp.devEUI = dev_EUI;
                    dev_tmp.devAppKey = dev_App_Key;
                    dev_tmp.devHash = dev_Hash;        //
                    dev_tmp.devAESKey = dev_AESKey;

                    dev_param_list[dev_param_list.Count - 1].param.Add(dev_tmp);    // добавить dev в год

                    cur_index_dev++;                 //номер от 1
                    num_cyr_year = cur_index_dev;    // последний добавленный по году!!
                }
            }

            int dev = dev_param_list.Count;
            LOG.log_str("loadDev: Загружены за " + dev_param_list.Count + " года");  //
            return true;
        }

    public static bool loadDev(string filename)
        {
            std::vector<string> str = getString(filename); //файл открывает и закрывает!  без коментов и лишних пробелов
            // есл файла нет??
            LOG.log_str("loadDev: Строк в " + filename + " = " + str.Count + " шт"); //
            return loadDevStr(str);

        }

        // ПЕРЕМЕННЫЕ И СТРУКТУРЫ для температуры
        // параметры коэфф. (косы ) полином- 9 коэф в одну строку для мас
    public class ds18b20_param     //надо создавать для каждого!
        {
        public string id;           //МАС
        public double a;         //
        public double b;        //
        public int position;        //
        public int parent_index;    //
        public double c;   //  ds_tmp.c = c1;   //доб. для полинома

        public double a2;   //ds_tmp.a2 = a2;
        public double b2;   //ds_tmp.b2 = b2;
        public double c2;   //ds_tmp.c2 = c2;
        public double a3;   //ds_tmp.a3 = a3;
        public double b3;   //ds_tmp.b3 = b3;
        public double c3;   //ds_tmp.c3 = c3;

        public ds18b20_param() // constructor
            {
                this->id = "";;           //МАС ?
                this->a = 0;         //
                this->b = 0; ;        //
                this->position = 0; ;        //
                this->parent_index = 0; ;    //
                this->c = 0; ;   //  ds_tmp.c = c1;   //доб. для полинома

                this->a2 = 0; ;   //ds_tmp.a2 = a2;
                this->b2 = 0; ;   //ds_tmp.b2 = b2;
                this->c2 = 0; ;   //ds_tmp.c2 = c2;
                this->a3 = 0; ;   //ds_tmp.a3 = a3;
                this->b3 = 0; ;   //ds_tmp.b3 = b3;
                this->c3 = 0; ;   //ds_tmp.c3 = c3;
            }
        }

    public class ds18b20_plume
        {
        public string Name;         //имя косы
        public std::vector<ds18b20_param> param;

        public ds18b20_plume()  // constructor
            {
                this->Name = "";
                this->param = new std::vector<ds18b20_param>();  //std::vector<ds18b20_param>
            }
        }

    public struct ds18b20_plume1         //для каждой новой косы в паспорте
        {
        public string Name;         //имя косы
            //надо создавать для каждого!
        public std::vector<ds18b20_param> param;     //МАС и коэф для нее static
        }

        //лист паспорта сам статик - обьекты нет!! надо новый!!
    public static std::vector<ds18b20_plume> Param_list = new std::vector<ds18b20_plume>();  //лист паспорта статик!! из классов

    public static bool load(Stream mstream, int type)
        {
            std::vector<string> str = getString(mstream);
            // далее все тоже что и в load(string filename, int type)
            // if (load_koef(str, type) == false) { return false; }
            return load_koef(str, type);
            //return load_koefAdd(str, type);
        }

    public static bool load(string filename, int type)
        {
            if (!File.Exists(filename))
            {
                LOG.log_str("load: Файл паспорта не найден! Файл: " + filename);  //
                return false;                //
            }
            std::vector<string> str = getString(filename);
            //  if (load_koef(str, type) == false) { return false; }
            return load_koef(str, type);       //
            //return load_koefAdd(str, type);
        }

        //добавить из файла без обнуления - типы 0 1 2
    public static bool loadAdd(string filename, int type)
        {
            if (!File.Exists(filename))
            {
                LOG.log_str("loadAdd: Файл паспорта не найден! Файл: " + filename);  //
                return false;                //
            }
            std::vector<string> str = getString(filename);
            //  if (load_koef(str, type) == false) { return false; }
            //return load_koef(str, type);       //
            return load_koefAdd(str, type);     //без обнуления!!
        }

        // подпрограмма чтения коэф. 3 шт !! std poly и angl только !! читает по одному !!!! по типу
        // добавлен тип новый инкл Angl
        //  чистить параметры!!! если тип не все All=5 ??
    public static bool load_koef(std::vector<string> str, int type)
        {
            //      if (Param_list == null)
            //      { LOG.log_str("load_koef: Param_list == null"); MessageBox.Show("Param_list == null"); return false; }

            Param_list.Clear();         // только по одному файлу!!!!!
            //dev_param_list.Clear();     // для лоры!!
            prog.kof_list.Clear();
            prog.koeff_list_angl.Clear();     //
            //prog.angl0id_list.Clear();
            LOG.log_str("load_koef: Param_list очищен ");
            return load_koefAdd(str, type);  // без стирания!!!
        }

        // без стирания Param_list !! и остальных!! для добавления просто!!
        // для паспорта aх b и полинома коэф a b наоборот!!!!
    public static bool load_koefAdd(std::vector<string> str, int type)
        {
            //        if (Param_list == null)
            //        { LOG.log_str("load_koef: Param_list == null"); MessageBox.Show("Param_list == null"); return false; }

            LOG.log_str("load_koefAdd: type = " + type);
            int cur_index = 0;              // кол.кос в паспорте
            bool ret = true;        //по умолчанию ошибок разбора нет
            String cyr_kosa = "";  //текущая коса для вывода ошибок

            // в листе мас нет поля для косы, +
            if (type == 0)                  // type == KoeffTypePASS
            {
                int index = 0;              //для koef индекс в паспорте!??
                string kosa = "";          //коса
                int number_kos = 0;                  // кол кос -для вывода

                LOG.log_str("load_koefAdd: str.Count = " + str.Count);
                for (int i = 0; i < str.Count; i++)
                {
                    if (str[i] == "") continue;  //
                    if (str[i][0] == 'N')
                    {
                        // находить NEND !!!
                        if (str[i] == "NEND") { break; }   // если короткое имя до 4 знаков!!
                        // конец файла, ставить в начале ! не попадает в паспорт!!

                        ds18b20_plume tmp = new ds18b20_plume();  //для косы создать
                        tmp.Name = str[i];   //коса для парам
                        tmp.param = new std::vector<ds18b20_param>();  //для мас создать
                        Param_list.Add(tmp);
                        cur_index++;
                        //для koef - новая коса
                        kosa = str[i];
                        index = 0;
                        number_kos++;
                    }
                    else  //
                    {
                        Double a = 1;       //ax  как в паспорте
                        Double b = 0;       //b
                        Double c = 0;       //cx**2
                        NumberFormatInfo nfi = new CultureInfo("en-US", false).NumberFormat;
                        nfi.NumberDecimalSeparator = ",";
                        nfi.NegativeSign = "-";
                        NumberStyles styl = NumberStyles.Float;  // может быть научный! -8,54764450e-002
                        Regex reg = new Regex(@"([0-9a-fA-F]+)\s+([0-9\,\.\-]+)\s+([0-9\,\.\-]+)");  // 2 коэф или ???  групп 3 + 1
                        Regex reg_mac = new Regex(@"([0-9a-fA-F]+)\s");  // c пробелом! \A	Соответствие должно обнаруживаться в начале строки.

                        Regex reg_koeff = new Regex(@"(\s[0-9\,\.\-]+){2,3}");    // без пробела! в конце но обязательно в начале!! {2,3} не менее 2 не более 3


                        string mac_code = "";

                        //базовый с мас и 2 коэф. в reg
                        Match m = reg.Match(str[i]);            //26F44A190100009B 1 0      4 группы!
                        int num_gr = m.Groups.Count;           //
                        //           if (Form1.fdbgpass == true) LOG.log_str("num_gr = " + num_gr);  //
                        if (m.Success)                      //при 0 и 1 даст ошибку при 2 и более - вернет 2 !!
                        {
                            //               if (Form1.fdbgpass == true) { LOG.log_str("Строка паспорта m = " + m.Value); } //
                            // mac_code определяется ниже еще раз !!!
                            mac_code = m.Groups[1].Value;    //должно быть всегда ???
                        }
                            //нет мас и 2-х коэф (если больше - возмет 2 !)
                        else
                        {
                            LOG.log_str("load_koefAdd: Паспорт (Match m стандартная) с ошибкой в косе: " + cyr_kosa);  // при 9 1 коэф  - исключения не будет !!!
                            //mac_code = "0000000000000000";  // пока пусто!!
                            LOG.log_str("load_koefAdd: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                            a = 1;
                            b = 0;
                            c = 0;
                            //return false;    // сразу выход с ошибкой !! дальше не грузит!!
                            ret = false;
                        }

                        //МАС проверять что есть с !!! или делать его =0
                        Match m1 = reg_mac.Match(str[i]);           //26F44A190100009B 2 группы только МАС
                        int num_gr1 = m1.Groups.Count;              // всегда=2 ??
                        //           if (Form1.fdbgpass == true) LOG.log_str("num_gr1 = " + num_gr1);  //
                        if (m1.Success)
                        {
                            int captureCtrMac;
                            for (int ctr = 1; ctr <= m1.Groups.Count - 1; ctr++)
                            {
                                //  LOG.log_str(String.Format("Group {0}:  {1}", ctr,m_name.Groups[ctr].Value));
                                int captureCtr = 0;
                                foreach (Capture capture in m1.Groups[ctr].Captures)  // заполнение
                                {
                                    // вывод всех
                                    //                      if (Form1.fdbgpass == true) LOG.log_str(String.Format("#mac Capture {0}: {1}", captureCtr, capture.Value));
                                    captureCtr += 1;
                                }
                                captureCtrMac = captureCtr;
                            }
                            String mac_cap = m1.Groups[1].Captures[0].Value;
                            mac_code = mac_cap;    ////mac! без пробела !!!!

                            //mac_code = m1.Value; //mac!  проверять на длину ??? и начальные 26 28 00 ??
                            //искл                       mac_code = m.Groups[1].Value;    //должно быть всегда ???
                            //             if (Form1.fdbgpass == true) LOG.log_str("m1 mac = " + m1.Value);  //mac! с пробелом !!!!

                            String sub = mac_code.Substring(0, 2);  // "00" для угла!??

                            if ((sub != "28") && (sub != "26") && (sub != "00")) //mac!  проверять на длину ??? и начальные 26 28 00 ??
                            {
                                LOG.log_str("load_koefAdd: Коса: " + cyr_kosa + " , MAC = " + mac_code);  //
                                LOG.log_str("load_koefAdd: Датчик не 26, 28, 00!! Начало = " + sub);  //mac!
                                ret = false;
                            }

                            if (mac_code.Length != 16)
                            {
                                LOG.log_str("load_koefAdd: Коса: " + cyr_kosa + " , MAC = " + mac_code);  //
                                LOG.log_str("load_koefAdd: Длина МАС адреса датчика не 16 байт! Длина = " + mac_code.Length);  //mac!
                                ret = false;
                            }
                        }
                        else LOG.log_str("load_koefAdd: Нет МАС в начале строки паспорта косы: " + cyr_kosa);  //
                        //===================================
                        //коэф только ! берет не более 3!! так в reg_koeff
                        Match m_name = reg_koeff.Match(str[i]);           //
                        int num_gr_name = m_name.Groups.Count;   //групп две всегда !!!!!!!!
                        //          if (Form1.fdbgpass == true) LOG.log_str("num_gr_name = " + num_gr_name);  //
                        if (m_name.Success)                      //при  1 даст ???
                        {
                            //              if (Form1.fdbgpass == true) LOG.log_str("m_name = " + m_name.Value);  //

                            //int group1CtrCap = 0;      //кол.групп - всегда 2 ??
                            if (num_gr_name == 2)
                            {
                                int captureCtr1 = 0;     //кол в гр1
                                for (int ctr = 1; ctr <= m_name.Groups.Count - 1; ctr++)
                                {
                                    //  LOG.log_str(String.Format("Group {0}:  {1}", ctr,m_name.Groups[ctr].Value));
                                    int captureCtr = 0;
                                    foreach (Capture capture in m_name.Groups[ctr].Captures)  // заполнение
                                    {
                                        // вывод всех
                                        //                      if (Form1.fdbgpass == true) LOG.log_str(String.Format("#Koeff Capture {0}: {1}", captureCtr, capture.Value));
                                        captureCtr += 1;
                                    }
                                    captureCtr1 = captureCtr;
                                }
                                // captureCtr кол коэф   значение в 0 1 и тд
                                if (captureCtr1 < 2)  // менее 2 х коэф       // int num_koeff = 0;
                                {
                                    LOG.log_str("load_koefAdd: Коэф менее 2 (Match m_name error). Коса: " + cyr_kosa);  // gr = 3 и менее
                                    LOG.log_str("load_koefAdd: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                    a = 1;
                                    b = 0;
                                    c = 0;
                                    ret = false;
                                }
                                else if (captureCtr1 == 2)   // коэф 2 шт
                                {
                                    try
                                    {
                                        a = Double.Parse(m_name.Groups[1].Captures[0].Value, styl, nfi);     //
                                        b = Double.Parse(m_name.Groups[1].Captures[1].Value, styl, nfi); //
                                    }
                                    catch (System.Exception)
                                    {
                                        try
                                        {
                                            a = Double.Parse(m_name.Groups[1].Captures[0].Value.Replace('.', ','), styl, nfi);  // для учета запятых или точек
                                            b = Double.Parse(m_name.Groups[1].Captures[1].Value.Replace('.', ','), styl, nfi);
                                            //                          if (Form1.fdbgpass == true) LOG.log_str("Запятые заменены на точки 2 коэф");  //
                                        }
                                        catch (System.Exception)
                                        {
                                            a = 1;
                                            b = 0;
                                            c = 0;
                                            LOG.log_str("load_koefAdd: 2 коэф. Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                            ret = false;
                                        }
                                    }
                                }
                                else   // коэф более 2 - берет 3 !!!! остальные выбрасывает!!
                                {
                                    try
                                    {
                                        a = Double.Parse(m_name.Groups[1].Captures[0].Value, styl, nfi);     //
                                        b = Double.Parse(m_name.Groups[1].Captures[1].Value, styl, nfi);     //
                                        c = Double.Parse(m.Groups[1].Captures[0].Value, styl, nfi);     //
                                    }
                                    catch (System.Exception)
                                    {
                                        try
                                        {
                                            a = Double.Parse(m_name.Groups[1].Captures[0].Value.Replace('.', ','), styl, nfi);  // для учета запятых или точек
                                            b = Double.Parse(m_name.Groups[1].Captures[1].Value.Replace('.', ','), styl, nfi);
                                            c = Double.Parse(m_name.Groups[1].Captures[2].Value.Replace('.', ','), styl, nfi);
                                            //                       if (Form1.fdbgpass == true) LOG.log_str("Запятые заменены на точки 3 коэф");  //
                                        }
                                        catch (System.Exception)
                                        {
                                            a = 1;
                                            b = 0;
                                            c = 0;
                                            LOG.log_str("load_koefAdd: 3 коэф. Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                            ret = false;
                                        }
                                    }
                                }
                            }
                            else // выход !!! с ошибкой gr>2
                            {
                                LOG.log_str("load_koefAdd: Групп более 2 (Match error). Коса: " + cyr_kosa);  // gr > 2 !
                                LOG.log_str("load_koefAdd: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                a = 1;
                                b = 0;
                                c = 0;
                                ret = false;
                            }
                        }
                        else  //
                        {
                            LOG.log_str("load_koefAdd: m_name: Нет коэф. в паспорте (Match m_name error) Коса: " + cyr_kosa);  //
                            LOG.log_str("load_koefAdd: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                            a = 1;
                            b = 0;
                            c = 0;
                            ret = false;
                        }
                        //        if (Form1.fdbgpass == true) LOG.log_str("Конец строки");  //
                        //========================
                        //заполнение парам
                        ds18b20_param ds_tmp = new ds18b20_param();
                        ds_tmp.position = Param_list[Param_list.Count - 1].param.Count; //  кол.МАС в паспорте
                        ds_tmp.id = mac_code;
                        ds_tmp.a = a;       //a= 1 по умолч. !!!! НАОБОРОТ a b !!! в полиноме
                        ds_tmp.b = b;
                        ds_tmp.c = c;                             //в ds18b20_param есть c !
                        ds_tmp.parent_index = Param_list.Count - 1; // номер в табл.коэф
                        Param_list[Param_list.Count - 1].param.Add(ds_tmp);

                        //заполнение коэф. kosa заполнена
                        //Single b  a c !!!!  a=a0=f b=a1=e c=a2=d  a3=c a4=b a5=a
                        //                 с конца!  для акон  a   b  c  d  e  f
                        prog.kof_list.Add(new Koeff(mac_code, 0, 0, 0, (Single)c, (Single)b, (Single)a, -274, -3, kosa, index));
                        index++;

                    }  // if N
                }  //for str Count
                LOG.log_str("load_koefAdd: End type 0");
                LOG.log_str("load_koefAdd: ret = " + ret);
                //  return ret;  // вернут если были ошибки разбора!!
                return true;
            }  //end type 0
            else if (type == 1) // polynom.txt в нем a b наоборот!!
            {
                int index = 0;          // для коэф
                string kosa = "";
                int number_kos = 0;     // кол кос -для вывода

                LOG.log_str("load_koefAdd: str.Count = " + str.Count);
                for (int i = 0; i < str.Count; i++)
                {
                    if (str[i] == "") continue;
                    if (str[i][0] == 'N')
                    {
                        if (str[i] == "NEND") { break; }   // если короткое имя до 4 знаков!!
                        ds18b20_plume tmp = new ds18b20_plume();
                        tmp.Name = str[i];                          // имя косы
                        tmp.param = new std::vector<ds18b20_param>();      //
                        Param_list.Add(tmp);
                        cur_index++;
                        //
                        kosa = str[i];
                        index = 0;
                        number_kos++;
                    }
                    else // для коэф
                    {
                        // 9 коэф - формат -2,97418926e-001 не берет!!!!!!!!!!
                        Regex reg = new Regex(@"([0-9a-fA-F]+)\s+([0-9\,\.\-\+eE]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)");
                        //отдельно мас и коэф
                        Regex reg_mac = new Regex(@"([0-9a-fA-F]+)\s");  // c пробелом!  \A	Соответствие должно обнаруживаться в начале строки.
                        Regex reg_koeff = new Regex(@"(\s[0-9\,\.\-\+eE]+){9}");    // без пробела! в конце но обязательно в начале!! {9} шт

                        NumberFormatInfo nfi = new CultureInfo("en-US", false).NumberFormat; //рег.параметры
                        nfi.NumberDecimalSeparator = ","; //
                        nfi.NegativeSign = "-";
                        NumberStyles styl = NumberStyles.Float;  // может быть научный! не берет!!!!!!!!!!

                        //              Double a = 0; Double b = 1; Double c = 0;
                        //              Double a2 = 0; Double b2 = 1; Double c2 = 0;
                        //              Double a3 = 0; Double b3 = 1; Double c3 = 0;

                        Single a = 0; Single b = 1; Single c = 0;
                        Single a2 = 0; Single b2 = 1; Single c2 = 0;
                        Single a3 = 0; Single b3 = 1; Single c3 = 0;

                        //               Single a, b, c, a2, b2, c2, a3, b3, c3;
                        string mac_code = "";
                        //===============
                        Match m = reg.Match(str[i]);    // как было
                        int num_gr = m.Groups.Count;    //
                        //        if (Form1.fdbgpass == true) LOG.log_str("num_gr = " + num_gr);  //
                        if (m.Success)                      //при 0 и 1 даст ошибку при 2 и более - вернет 2 !!
                        {
                            //             if (Form1.fdbgpass == true) LOG.log_str("Строка паспорта m = " + m.Value);  //
                            mac_code = m.Groups[1].Value;    //должно быть всегда ???
                        }
                            //нет мас и 2-х коэф (если больше - возмет 2 !)
                        else
                        {
                            LOG.log_str("Паспорт (Match m стандартная) с ошибкой в косе: " + cyr_kosa);  // при 9 1 коэф  - исключения не будет !!!
                            //mac_code = "0000000000000000";
                            LOG.log_str("Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                            a = 0;
                            b = 1;
                            c = 0;
                            //return false;    // сразу выход с ошибкой !! дальше не грузит!!
                            ret = false;
                        }

                        // string mac_code = m.Groups[1].Value;  // как было
                        //======================
                        // Match m_mac = reg_mac.Match(str[i]);    //отдельно мас

                        // проверять что есть с !!! или делать его =0
                        Match m_mac = reg_mac.Match(str[i]);           //26F44A190100009B 2 группы
                        int num_gr1 = m_mac.Groups.Count;              // всегда=2 ??
                        //         if (Form1.fdbgpass == true) LOG.log_str("num_gr1 = " + num_gr1);  //
                        if (m_mac.Success)
                        {
                            int captureCtrMac;
                            for (int ctr = 1; ctr <= m_mac.Groups.Count - 1; ctr++)
                            {
                                //  LOG.log_str(String.Format("Group {0}:  {1}", ctr,m_name.Groups[ctr].Value));
                                int captureCtr = 0;
                                foreach (Capture capture in m_mac.Groups[ctr].Captures)  // заполнение
                                {
                                    // вывод всех
                                    //                    if (Form1.fdbg == true) LOG.log_str(String.Format("#mac Capture {0}: {1}", captureCtr, capture.Value));
                                    captureCtr += 1;
                                }
                                captureCtrMac = captureCtr;
                            }
                            String mac_cap = m_mac.Groups[1].Captures[0].Value;
                            mac_code = mac_cap;    //mac! без пробела !!!!

                            //mac_code = m1.Value; //mac!  проверять на длину ??? и начальные 26 28 00 ??
                            //искл                       mac_code = m.Groups[1].Value;    //должно быть всегда ???
                            //            if (Form1.fdbgpass == true) LOG.log_str("m_mac = " + m_mac.Value);  //mac! с пробелом !!!!

                            String sub = mac_code.Substring(0, 2);

                            if ((sub != "28") && (sub != "26") && (sub != "00")) //mac!  проверять на длину ??? и начальные 26 28 00 ??
                            {
                                LOG.log_str("Коса: " + cyr_kosa + " , MAC = " + mac_code);  //
                                LOG.log_str("Датчик не 26, 28, 00!! Начало = " + sub);  //mac!
                                ret = false;
                            }

                            if (mac_code.Length != 16)
                            {
                                LOG.log_str("Коса: " + cyr_kosa + " , MAC = " + mac_code);  //
                                LOG.log_str("Длина МАС адреса датчика не 16 байт! Длина = " + mac_code.Length);  //mac!
                                ret = false;
                            }
                        }
                        else LOG.log_str("Нет МАС в начале строки паспорта косы: " + cyr_kosa);  //

                        //=======================================
                        //коэф только ! берет  9!! так в reg_koeff
                        Match m_koeff = reg_koeff.Match(str[i]);     //отдельно коэф
                        int num_gr_koeff = m_koeff.Groups.Count;   //групп две всегда !!!!!!!!
                        //        if (Form1.fdbgpass == true) LOG.log_str("num_gr_name = " + num_gr_koeff);  //
                        if (m_koeff.Success)                      //при  1 даст ???
                        {
                            //            if (Form1.fdbgpass == true) LOG.log_str("m_name = " + m_koeff.Value);  //

                            if (num_gr_koeff == 2)      //кол.групп - всегда 2 ??
                            {
                                int captureCtr1 = 0;     //кол в гр1
                                for (int ctr = 1; ctr <= m_koeff.Groups.Count - 1; ctr++)
                                {
                                    //  LOG.log_str(String.Format("Group {0}:  {1}", ctr,m_koeff.Groups[ctr].Value));
                                    int captureCtr = 0;
                                    foreach (Capture capture in m_koeff.Groups[ctr].Captures)  // заполнение
                                    {
                                        // вывод всех
                                        //                     if (Form1.fdbg == true) LOG.log_str(String.Format("#Koeff Capture {0}: {1}", captureCtr, capture.Value));
                                        captureCtr += 1;
                                    }
                                    captureCtr1 = captureCtr;
                                }
                                // captureCtr кол коэф   значение в 0 1 и тд
                                if (captureCtr1 != 9)  // не 9  коэф       //
                                {
                                    LOG.log_str("Коэф не равно 9 (Match m_name error). Коса: " + cyr_kosa);  // gr = 3 и менее
                                    LOG.log_str("Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                    a = 0; a2 = 0; a3 = 0;
                                    b = 1; b2 = 1; b3 = 1;
                                    c = 0; c2 = 0; c3 = 0;
                                    ret = false;
                                }
                                else // if (captureCtr1 == 9)   // коэф  9 и вид -2,97418926e-001 не брал!!??
                                {

                                    try // T = a + b*t + c*t**2
                                    {

                                        a = Single.Parse(m_koeff.Groups[1].Captures[0].Value, styl, nfi); //
                                        b = Single.Parse(m_koeff.Groups[1].Captures[1].Value, styl, nfi); //
                                        c = Single.Parse(m_koeff.Groups[1].Captures[2].Value, styl, nfi); //
                                        a2 = Single.Parse(m_koeff.Groups[1].Captures[3].Value, styl, nfi); //
                                        b2 = Single.Parse(m_koeff.Groups[1].Captures[4].Value, styl, nfi); //
                                        c2 = Single.Parse(m_koeff.Groups[1].Captures[5].Value, styl, nfi); //
                                        a3 = Single.Parse(m_koeff.Groups[1].Captures[6].Value, styl, nfi); //
                                        b3 = Single.Parse(m_koeff.Groups[1].Captures[7].Value, styl, nfi); //
                                        c3 = Single.Parse(m_koeff.Groups[1].Captures[8].Value, styl, nfi); //
                                    }
                                    catch (System.Exception)  // в строках замена
                                    {
                                        try
                                        {

                                            a = Single.Parse(m_koeff.Groups[1].Captures[0].Value.Replace('.', ','), styl, nfi);  // для учета запятых или точек
                                            b = Single.Parse(m_koeff.Groups[1].Captures[1].Value.Replace('.', ','), styl, nfi);
                                            c = Single.Parse(m_koeff.Groups[1].Captures[2].Value.Replace('.', ','), styl, nfi);
                                            a2 = Single.Parse(m_koeff.Groups[1].Captures[3].Value.Replace('.', ','), styl, nfi);  // для учета запятых или точек
                                            b2 = Single.Parse(m_koeff.Groups[1].Captures[4].Value.Replace('.', ','), styl, nfi);
                                            c2 = Single.Parse(m_koeff.Groups[1].Captures[5].Value.Replace('.', ','), styl, nfi);
                                            a3 = Single.Parse(m_koeff.Groups[1].Captures[6].Value.Replace('.', ','), styl, nfi);  // для учета запятых или точек
                                            b3 = Single.Parse(m_koeff.Groups[1].Captures[7].Value.Replace('.', ','), styl, nfi);
                                            c3 = Single.Parse(m_koeff.Groups[1].Captures[8].Value.Replace('.', ','), styl, nfi);
                                            //                       if (Form1.fdbg == true) LOG.log_str("Запятые заменены на точки 9 коэф");  //
                                        }
                                        catch (System.Exception)  // совсем ошибка
                                        {
                                            a = 0; a2 = 0; a3 = 0;
                                            b = 1; b2 = 1; b3 = 1;
                                            c = 0; c2 = 0; c3 = 0;
                                            LOG.log_str("2 коэф. Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                            ret = false;
                                        }
                                    }
                                }
                            }
                            else // выход !!! с ошибкой gr>2
                            {
                                LOG.log_str("Групп более 2 (Match error). Коса: " + cyr_kosa);  // gr > 2 !
                                LOG.log_str("Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                a = 0; a2 = 0; a3 = 0;
                                b = 1; b2 = 1; b3 = 1;
                                c = 0; c2 = 0; c3 = 0;
                                ret = false;
                            }
                        }
                        else  //
                        {
                            LOG.log_str("m_name: Нет коэф. в паспорте (Match m_name error) Коса: " + cyr_kosa);  //
                            LOG.log_str("Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                            a = 0; a2 = 0; a3 = 0;
                            b = 1; b2 = 1; b3 = 1;
                            c = 0; c2 = 0; c3 = 0;
                            ret = false;
                        }
                        //             if (Form1.fdbgpass == true) LOG.log_str("Конец строки");  //

                        ds18b20_param ds_tmp = new ds18b20_param();
                        ds_tmp.position = Param_list[Param_list.Count - 1].param.Count;
                        ds_tmp.id = mac_code;

                        ds_tmp.a = (Double)a;          // T = a + b*t + c*t**2 для паспорта a b наоборот!
                        ds_tmp.b = (Double)b;
                        ds_tmp.c = (Double)c;       // для полинома

                        ds_tmp.a2 = (Double)a2;
                        ds_tmp.b2 = (Double)b2;
                        ds_tmp.c2 = (Double)c2;
                        ds_tmp.a3 = (Double)a3;
                        ds_tmp.b3 = (Double)b3;
                        ds_tmp.c3 = (Double)c3;
                        ds_tmp.parent_index = Param_list.Count - 1;
                        Param_list[Param_list.Count - 1].param.Add(ds_tmp);

                        //kosa заполнена
                        //Single a b c !!!!
                        //                              a   b  c   d  e   f
       prog.kof_list.Add(new Koeff(mac_code, 0, 0, 0, (Single)c, (Single)b, (Single)a, -274, -3, kosa, index));
                        prog.kof_list.Add(new Koeff(mac_code, 0, 0, 0, (Single)c2, (Single)b2, (Single)a2, -3, 3, kosa, index));
                        prog.kof_list.Add(new Koeff(mac_code, 0, 0, 0, (Single)c3, (Single)b3, (Single)a3, 3, 500, kosa, index));
                        index++;
                        cur_index++;
                    }//else N
                } //for str Count
                LOG.log_str("load_koefAdd: End type 1");
                LOG.log_str("load_koefAdd: ret = " + ret);
                //  return ret;  // вернут если были ошибки разбора!!
                return true;

            } // end type 1
                //kx0off, ky0off, kz0off, kxs, kys, kzs  //для angl  angl_pas.txt
                // темп как для паспорта a b
            else if (type == 2)      //angl_pas.txt для нового инкл!! type == KoeffTypeXYZ для углов и KoeffType для темп
            {
                // файл уже прочитан
                //  for (int i = 0; i < Passport_file.Count; i++)       // кол.строк
                string kosa = "";                    // имя косы
                int index = 0;                     // индекс мас в косе - с 0
                int indexi = 0;                      // для коэф угла индекс мас в косе
                int number_kos = 0;                  // кол кос -для вывода

                LOG.log_str("load_koefAdd: str.Count = " + str.Count);
                for (int i = 0; i < str.Count; i++)
                {
                    // string str = Passport_file[i];
                    string st = str[i];
                    if (st.Length == 0) { continue; }
                    if (st[0] == 'N')               // самый первый символ !! без пробелов!!! только одна первая строка с 1
                    {
                        if (st == "NEND") { break; }   // если короткое имя до 4 знаков!!
                        // заполнение для темп в парам лист!!
                        ds18b20_plume tmp = new ds18b20_plume();
                        tmp.Name = str[i];                          // имя косы
                        tmp.param = new std::vector<ds18b20_param>();      //
                        Param_list.Add(tmp);
                        cur_index++;
                        //для коэф
                        kosa = st;          // имя косы
                        index = 0;         // индекс в паспорте
                        indexi = 0;
                        number_kos++;   // NEND не входит!?? выход раньше!
                    }
                    else  //
                    {
                        //всего до 7!! (МАС+6) проверять на мас адрес для темп отдельно - пока 2 коэф. Для ММА Коэф обязательно 6 !!!!!!!!
                        // вначале МАС потом коэф. - это для ММА и темп ВСЕ РАВНО ПО 6 ЦИФР !!!!!!
                        // a-fA-F для МАС далее только цифры!? 1          2                    3                4                     5                  6                    7
                        Regex reg = new Regex(@"([0-9a-fA-F]+)\s+([0-9\,\.\-\+eE]+)\s+([0-9\,\.\-\+eE]+)\s+([0-9\,\.\-\+eE]+)\s+([0-9\,\.\-\+eE]+)\s+([0-9\,\.\-\+eE]+)\s+([0-9\,\.\-\+eE]+)");
                        Match m = reg.Match(st);
                        string mac_code = m.Groups[1].Value;        // дает пустую строку?? при 00 МАС ??
                        //string mac_code = m.Groups[1].Value.ToString();           //
                        NumberFormatInfo nfi = new CultureInfo("en-US", false).NumberFormat;
                        nfi.NumberDecimalSeparator = ",";
                        nfi.NegativeSign = "-";
                        NumberStyles styl = NumberStyles.Float;  // может быть научный!

                        //может дать  дает исключение по длине !!!!!!!!!!!!!!!!
                        //если коэф.менее 6 даже для датчиков темп. !!!!! ДОЛЖНО СОВПАДАТЬ КОЛИЧЕСТВО и разделитель запятая!!!
                        if (mac_code.Length < 2)        //
                        {
//                           MessageBox.Show("МАС = " + mac_code + " : МАС менее 2 символов!", "", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            // ошибка в чтении МАС ??
                            //               return 0;            // выйдет с ошибкой длины!
                            //LOG.log_str("МАС = " + mac_code + " : МАС менее 2 символов!"); ret = false;
                            return false;
                        }
                        // prog.MMA_CODE
                        if (mac_code.Substring(0, 2) == "00")   //00 для датчика углов
                        {
                            //kx0off, ky0off, kz0off, kxs, kys, kzs
                            Single k0x, k0y, k0z, kx, ky, kz;
                            try
                            {
                                // здесь должен брать коэф. с точкой !! или запятой - как установки компа!! nfi = , - как в руском!
                                k0x = Single.Parse(m.Groups[2].Value, styl, nfi);
                                k0y = Single.Parse(m.Groups[3].Value, styl, nfi);
                                k0z = Single.Parse(m.Groups[4].Value, styl, nfi);
                                kx = Single.Parse(m.Groups[5].Value, styl, nfi);
                                ky = Single.Parse(m.Groups[6].Value, styl, nfi);
                                kz = Single.Parse(m.Groups[7].Value, styl, nfi);
                            }
                            catch (System.Exception) //
                            {
                                try
                                {
                                    //kx0off, ky0off, kz0off, kxs, kys, kzs
                                    k0x = Single.Parse(m.Groups[2].Value.Replace(',', '.'), styl, nfi);
                                    k0y = Single.Parse(m.Groups[3].Value.Replace(',', '.'), styl, nfi);
                                    k0z = Single.Parse(m.Groups[4].Value.Replace(',', '.'), styl, nfi);
                                    kx = Single.Parse(m.Groups[5].Value.Replace(',', '.'), styl, nfi);
                                    ky = Single.Parse(m.Groups[6].Value.Replace(',', '.'), styl, nfi);
                                    kz = Single.Parse(m.Groups[7].Value.Replace(',', '.'), styl, nfi);
                                    //return (900010 + 3);  // была замена запятых на точки!!
                                    //              if (fdebug) MessageBox.Show("В коэф. для МАС: " + mac_code + " была замена запятых на точки!!", "", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                    //LOG.log_str("В коэф. для МАС: " + mac_code + " была замена запятых на точки!!"); ret = false;
                                }
                                catch (System.Exception)
                                {
                                    //kx0off, ky0off, kz0off, kxs, kys, kzs
                                    k0x = (Single)0;
                                    k0y = (Single)0;
                                    k0z = (Single)0;
                                    kx = (Single)0.02197265625;  // чувствительность по каналу
                                    ky = (Single)0.02197265625;
                                    kz = (Single)0.02197265625;
                                    //                if (fdebug) MessageBox.Show("Коэф. для МАС: " + mac_code + " взяты по умолчанию!", "", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                    //LOG.log_str("Коэф. для МАС: " + mac_code + " взяты по умолчанию!"); ret = false;
                                    //return (900010 + 2);  // коэфф. по умолчанию!!
                                }
                            }
                            //public void KoeffAdd(string nmac, Single x0off, Single y0off, Single z0off, Single xs, Single ys, Single zs, string nkosa, int nIndex)
                            // мас с 00
//дополнить!!!!
                            prog.koeff_list_angl.Add(new KoeffXYZ(mac_code, k0x, k0y, k0z, kx, ky, kz, kosa, indexi));
                            indexi++;
                        }
                            //prog.DS2438_CODE prog.DS18B20_CODE
                            //mac_code, a, b, 0, 0, 0, 0, последние 4 не берет!!
                            //mac_code, a, b, c,
                            //   else if ((mac_code.Substring(0, 2) == DS18B20_CODE) || (mac_code.Substring(0, 2) == DS2438_CODE))     //МАС и  2 koeff !!! для темп.
                        else if ((mac_code.Substring(0, 2) == "28") || (mac_code.Substring(0, 2) == "26"))
                        {
                            Double a = 1; Double b = 0; Double c = 0;
                            //   Single a, b, c;
                            try
                            {
                                a = Double.Parse(m.Groups[2].Value, styl, nfi);
                                b = Double.Parse(m.Groups[3].Value, styl, nfi);
                                c = Double.Parse(m.Groups[4].Value, styl, nfi);
                            }
                            catch (System.Exception)
                            {
                                try
                                {
                                    a = Double.Parse(m.Groups[2].Value.Replace(',', '.'), styl, nfi);
                                    b = Double.Parse(m.Groups[3].Value.Replace(',', '.'), styl, nfi);
                                    c = Double.Parse(m.Groups[4].Value.Replace(',', '.'), styl, nfi);
                                }
                                catch (System.Exception)
                                {
                                    a = (Double)1;
                                    b = (Double)0;
                                    c = (Double)0;
                                    //LOG.log_str("Коэф. для МАС: " + mac_code + " взяты по умолчанию!"); ret = false;
                                }
                            }
                            //дополнено !! в Param_list тоже пишет!! только датчики темп!
                            //для датчиков темп.! только
                            ds18b20_param ds_tmp = new ds18b20_param();
                            ds_tmp.position = Param_list[Param_list.Count - 1].param.Count; //  кол.МАС в паспорте
                            ds_tmp.id = mac_code;
                            ds_tmp.a = a;
                            ds_tmp.b = b;
                            ds_tmp.c = c;                             //в ds18b20_param есть c !
                            ds_tmp.parent_index = Param_list.Count - 1; // номер в табл.коэф
                            Param_list[Param_list.Count - 1].param.Add(ds_tmp);

                            //для с ?????
                            //                              a   b  c   d  e   f
                            //    kof_list.Add(new Koeff(mac_code, 0, 0, 0, 0, a, b, -100, +125, kosa, index));
//дополнить
                            prog.kof_list.Add(new Koeff(mac_code, 0, 0, 0, (Single)c, (Single)a, (Single)b, -100, +125, kosa, index));

                            //для АКОН
                            //порядок обратный f=a0 e=a1 d=a2 c=a3 и тд
                            //kof_list.Add(new Koeff(mac_code, a, b, c, d, e, f, st, et, kosa, index));
                            //для старого инкл ???
                            prog.koeff_list_angl.Add(new KoeffXYZ(mac_code, (Single)a, (Single)b, 0, 0, 0, 0, kosa, index));
                            index++;
                        }  // mac 26 28
                        else               // ошибка в чтении МАС ??
                        {
//                            MessageBox.Show("Неверный MAC в файле паспорта (не 00 или 26 28): " + mac_code + " ", "", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            //    return 0;            //
                            return false;
                            //Console.WriteLine("Неверный MAC в файле паспорта (не 00 или 26 28): " + mac_code); ret = false;
                            //был неверный MAC + номер строки ?
                        }
                    }  //if (str[0] != 'N')
                }   // Passport_file.Count;  str.Count
                //number_kos = number_kos - 1;        // NEND тоже входит в колич.!!??

                // return Passport_file.Count;
                LOG.log_str("load_koefAdd: type = " + type + " загружен");
                LOG.log_str("load_koefAdd: для типа 2 Кос = " + number_kos);
                LOG.log_str("load_koefAdd: ret = " + ret);
                //  return ret;  // вернут если были ошибки разбора!!
                return true;
            }  // end type 2
            else  //ошибка типа !! не 0 1 2
            {
                LOG.log_str("load_koefAdd: type = " + type);
//                MessageBox.Show("Не известный файл паспорта - нет в перечислении!");
                // return 0;
                return false;
            }
            // return true;
        }

    public static ds18b20_param search_param(string rom)
        {
            ds18b20_param empty = new ds18b20_param();
            empty.id = "NA";

            // if(type == 0)
            empty.a = 1;
            empty.b = 0;

            // else if(type == 1)
            ///}
            //empty.a = 0;
            //empty.b = 1;
            //empty.c = 0;               // T = a + b*t + c*t**2
            ///}

            for (int i = 0; i < Param_list.Count; i++)
            {
                for (int j = 0; j < Param_list[i].param.Count; j++)
                {
                    if (String.Equals(Param_list[i].param[j].id, rom, StringComparison.OrdinalIgnoreCase))
                    {
                        return Param_list[i].param[j];
                    }
                }
            }
            return empty;       //
        }

    public static ds18b20_param search_param_poly(string rom)
        {
            ds18b20_param empty = new ds18b20_param();
            empty.id = "NA";

            // if(type == 0)
            //empty.a = 1;
            //empty.b = 0;

            // else if(type == 1)
            ///}
            empty.a = 0;
            empty.b = 1;
            empty.c = 0;               // T = a + b*t + c*t**2
            empty.a2 = 0;
            empty.b2 = 1;
            empty.c2 = 0;
            empty.a3 = 0;
            empty.b3 = 1;
            empty.c3 = 0;

            ///}

            for (int i = 0; i < Param_list.Count; i++)
            {
                for (int j = 0; j < Param_list[i].param.Count; j++)
                {
                    if (String.Equals(Param_list[i].param[j].id, rom, StringComparison.OrdinalIgnoreCase))
                    {
                        return Param_list[i].param[j];
                    }
                }
            }
            return empty;       //
        }

    public static ds18b20_param search_param(int plume, string rom)
        {
            ds18b20_param empty = new ds18b20_param();
            empty.id = "NA";

            // if(type == 0)
            empty.a = 1;
            empty.b = 0;

            // else if(type == 1)
            ///{
            //empty.a = 0;
            //empty.b = 1;
            //empty.c = 0;               // T = a + b*t + c*t**2
            ///}

            for (int j = 0; j < Param_list[plume].param.Count; j++)
            {
                if (String.Equals(Param_list[plume].param[j].id, rom, StringComparison.OrdinalIgnoreCase))
                {
                    return Param_list[plume].param[j];
                }
            }
            return empty;       //
        }

    public static ds18b20_param search_param_poly(int plume, string rom)
        {
            ds18b20_param empty = new ds18b20_param();
            empty.id = "NA";

            // if(type == 0)
            //empty.a = 1;
            //empty.b = 0;

            // else if(type == 1)
            //{
            empty.a = 0;
            empty.b = 1;
            empty.c = 0;               // T = a + b*t + c*t**2
            empty.a2 = 0;
            empty.b2 = 1;
            empty.c2 = 0;
            empty.a3 = 0;
            empty.b3 = 1;
            empty.c3 = 0;
            //}

            for (int j = 0; j < Param_list[plume].param.Count; j++)
            {
                if (String.Equals(Param_list[plume].param[j].id, rom, StringComparison.OrdinalIgnoreCase))
                {
                    return Param_list[plume].param[j];
                }
            }
            return empty;       //
        }

        //    public bool check_id(string rom)
        ///    {
        //        if (search_param(rom).id != "NA")  {  return true;  }
        //        return false;
        ///    }
    public static bool check_id(string rom)
        {
            if (search_param_all(rom).id != "NA") { return true; }
            return false;
        }

    public static double compute(string rom, double raw_temp)
        {
            ds18b20_param param;
            param = search_param(rom);
            if (param.id != "NA")
            {
                // if(type == 0)
                return Math.Round(param.a * raw_temp + param.b, 3);

                // else if(type == 1)                   // T = a + b*t + c*t**2
                ///{
                // if(raw_temp <= -3)
                //return Math.Round(param.a + raw_temp * param.b + param.c * raw_temp * raw_temp, 3);
                // if((raw_temp > -3) && (raw_temp < 3))
                //return Math.Round(param.a2 + raw_temp * param.b2 + param.c2 * raw_temp * raw_temp, 3);
                // if(raw_temp >= 30)
                //return Math.Round(param.a3 + raw_temp * param.b3 + param.c3 * raw_temp * raw_temp, 3);
                ///}
            }
            return raw_temp;        //
        }
    public static double compute_poly(string rom, double raw_temp)
        {
            ds18b20_param param;
            param = search_param(rom);
            if (param.id != "NA")
            {
                // if(type == 0)
                //return Math.Round(param.a * raw_temp + param.b, 3);

                // else if(type == 1)                   // T = a + b*t + c*t**2
                ///{
                if(raw_temp < -3)
                    return Math.Round(param.a + raw_temp * param.b + param.c * raw_temp * raw_temp, 3);
                if((raw_temp >= -3) && (raw_temp <= 3))
                    return Math.Round(param.a2 + raw_temp * param.b2 + param.c2 * raw_temp * raw_temp, 3);
                if(raw_temp > 30)
                    return Math.Round(param.a3 + raw_temp * param.b3 + param.c3 * raw_temp * raw_temp, 3);
                ///}
            }
            return raw_temp;        //
        }
    public static double compute(int plume, string rom, double raw_temp)
        {
            ds18b20_param param;
            param = search_param(plume,rom);
            if (param.id != "NA")
            {
                // if(type == 0)
                return Math.Round(param.a * raw_temp + param.b, 3);
                // else if(type == 1)
                ///{
                // if(raw_temp <= -3)
                //return Math.Round(param.a + raw_temp * param.b + param.с * raw_temp * raw_temp, 3);
                // if((raw_temp > -3) && (raw_temp < 3))
                //return Math.Round(param.a2 + raw_temp * param.b2 + param.с2 * raw_temp * raw_temp, 3);
                // if(raw_temp >= 30)
                //return Math.Round(param.a3 + raw_temp * param.b3 + param.с3 * raw_temp * raw_temp, 3);
                ///}
            }
            return raw_temp;        //
        }
    public static double compute_poly(int plume, string rom, double raw_temp)
        {
            ds18b20_param param;
            param = search_param_poly(plume, rom);
            if (param.id != "NA")
            {
                // if(type == 0)
                //return Math.Round(param.a * raw_temp + param.b, 3);
                // else if(type == 1)
                ///{
                if(raw_temp < -3)
                    return Math.Round(param.a + raw_temp * param.b + param.c * raw_temp * raw_temp, 3);
                if((raw_temp >= -3) && (raw_temp <= 3))
                    return Math.Round(param.a2 + raw_temp * param.b2 + param.c2 * raw_temp * raw_temp, 3);
                if(raw_temp > 30)
                    return Math.Round(param.a3 + raw_temp * param.b3 + param.c3 * raw_temp * raw_temp, 3);
                ///}
            }
            return raw_temp;        //
        }

    public static string plume_name(string rom)
        {
            ds18b20_param param;
            param = search_param(rom);
            if (param.id != "NA")
            {
                return Param_list[param.parent_index].Name;
            }
            return "Нет Данных";
        }
    public static int plume_index(string rom)
        {
            ds18b20_param param;
            param = search_param(rom);
            if (param.id != "NA")
            {
                return param.parent_index;
            }
            return -1;
        }


        //--------------------- работа со всеми паспортами -------------------------------------------------

        //через load() и loadAdd() из текста файлов в основной!!
        //последовательное чтение трех последовательно!! файлов с темп. коэф
    public static bool load_all_passNew()
        {
            bool ret = true;
            bool ret1 = true;
            bool clear = false;
            //надо в один Param_list - его не обнулять ! при втором чтении
            if (File.Exists("passport.txt")) //с чисткой
            {
                ret = load("passport.txt", 0); clear = true;
                if (ret == false) { LOG.log_str("load_all_pass: passport.txt с ошибками"); } //
            }
            //else
            if (File.Exists("polynom.txt"))  //без чистки
            {
                if (clear) ret1 = loadAdd("polynom.txt", 1); //без чистки
                else { ret1 = load("polynom.txt", 1); clear = true; }   //с чисткой
                if (ret1 == false) { LOG.log_str("load_all_pass: polynom.txt с ошибками"); } //
            }
            //else
            if (File.Exists("angl_pas.txt"))  //без чистки для углов!
            {
                if (clear) ret = loadAdd("angl_pas.txt", 2);  //без чистки
                else { ret = load("angl_pas.txt", 2); clear = true; }   //с чисткой
                if (ret == false) { LOG.log_str("load_all_pass: angl_pas.txt с ошибками"); } //
            }
            if ((ret == false) || (ret1 == false)) { return false; }
            return true;
        }

//-------------------------- удалить НЕ ИСПОЛЬЗОЮТСЯ БОЛЕЕ ????????????????????
    public static bool load_all_passStream(Stream instream, Stream instream1)   //без this это статик! вход не как файлы а как потоки???
        {
            //    std::vector<string> str = getStringStreem(mstream);  // поток памяти
            //из листа байт массив и потом в поток памяти и наоборот
            // public std::vector<string> getStringStreem(this Stream instream) //из любого потока в лист
            //вызов
            //public bool load_all_pass(getStringStreem(this Stream instream), getStringStreem(this Stream instream1));
            return load_all_passList(getString(instream), getString(instream1));
        }

    public static bool load_all_pass_old()
        {
            // в initprg сейчас заполняет классы для файлов

            //надо в один Param_list - его не обнулять ! при втором чтении
            //if (File.Exists("passport.txt")) { load("passport.txt", 0); }  //с чисткой
            //if (File.Exists("polynom.txt")) {  loadAdd("polynom.txt", 1); }   //без чистки
            //if (File.Exists("angl_pas.txt")) { loadAdd("angl_pas.txt", 2);  } //без чистки для углов!

            //можно убрать! листы всегда есть
            if (!File.Exists("passport.txt") || !File.Exists("polynom.txt"))
            {
                //if (Form1.fdbg == true)
                LOG.log_str("load_all_pass: Одного Файла паспорта не найдено!");  //
                return false;
            }

            std::vector<string> str = new  std::vector<string>();
            if (File.Exists("passport.txt"))
            {
                str = getString("passport.txt");
                LOG.log_str("load_all_pass: Строк в passport.txt " + str.Count + " шт"); // 37 шт
            }
            else str.Add("");

            //polynom читать сразу
            std::vector<string> strp = new  std::vector<string>();
            if (!File.Exists("polynom.txt"))
            {
                strp = getString("polynom.txt");
                LOG.log_str("load_all_pass: Строк в polynom.txt " + strp.Count + " шт"); // 190 шт
            }
            else strp.Add("");

            // load_all_passList(std::vector<string> str, std::vector<string> strp);
            return load_all_passList(str, strp);
        }

        //2 листа для чтения std poly сразу - инклинометров и акона нет!!!
        // коэфф всего 3!!!! - как лоад но последовательно 2 типа (файла)
    public static bool load_all_passList(std::vector<string> str, std::vector<string> strp)
        {
            //LOG.log_str("load_all_passList: ");  //
            // сдесь уже листы
            Param_list.Clear();            // чистит в начале!!
            int cur_index = 0;              // кол. строк датчиков кос в обоих паспортах?

            bool ret = true;  //
            String cyr_kosa = "";  //текущая коса для вывода ошибок

            for (int i = 0; i < str.Count; i++)  // по кол.строк
            {
                int index = 0;
                string kosa = "";

                if (str[i] == "") continue;  //
                //    if (str[i] == "NEND") { break; }   // если короткое имя до 4 знаков!!
                if (str[i][0] == 'N')
                {
                    // находить NEND !!! может быть длина после N всего 1!!
                    // конец файла, ставить в начале ! не попадает в паспорт!!
                    if (str[i] == "NEND") { break; }   // если короткое имя до 4 знаков!!
                    ds18b20_plume tmp = new ds18b20_plume();
                    tmp.Name = str[i];                          // будет отд коса с именем NEND !!! пустая
                    tmp.param = new std::vector<ds18b20_param>();
                    Param_list.Add(tmp);                    // добавление паспорта косы в Param_list
                    cur_index++;
                    //
                    kosa = str[i];
                }
                else  // 2 koeff было!
                {
                    Double a = 1;
                    Double b = 0;
                    Double c = 0;
                    NumberFormatInfo nfi = new CultureInfo("en-US", false).NumberFormat;
                    nfi.NumberDecimalSeparator = ",";
                    nfi.NegativeSign = "-";
                    Regex reg = new Regex(@"([0-9a-fA-F]+)\s+([0-9\,\.\-]+)\s+([0-9\,\.\-]+)");  // 2 коэф или ???  групп 3 + 1

                    // искать сначала мас один - первый! а потом коф все или 2 или 3 !!
                    Regex reg_mac = new Regex(@"([0-9a-fA-F]+)\s");  // c пробелом!  \A	Соответствие должно обнаруживаться в начале строки.

                    Regex reg_koeff = new Regex(@"(\s[0-9\,\.\-]+){2,3}");    // без пробела! в конце но обязательно в начале!! {2,3} не менее 2 не более 3


                    string mac_code = "";

                    //базовый
                    Match m = reg.Match(str[i]);            //26F44A190100009B 1 0      4 группы!
                    int num_gr = m.Groups.Count;           //
                    //       if (Form1.fdbgpass == true) LOG.log_str("num_gr = " + num_gr);  //
                    if (m.Success)                      //при 0 и 1 даст ошибку при 2 и более - вернет 2 !!
                    {
                        //          if (Form1.fdbgpass == true) { LOG.log_str("Строка паспорта m = " + m.Value); } //
                        // mac_code определяется ниже еще раз !!!
                        mac_code = m.Groups[1].Value;    //должно быть всегда ???
                    }
                        //нет мас и 2-х коэф (если больше - возмет 2 !)
                    else
                    {
                        //LOG.log_str("load_all_passList: ");  //
                        LOG.log_str("load_all_passList: Паспорт (Match m стандартная) с ошибкой в косе: " + cyr_kosa);  // при 9 1 коэф  - исключения не будет !!!
                        //mac_code = "0000000000000000";  // пока пусто!!
                        LOG.log_str("load_all_passList: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                        a = 1;
                        b = 0;
                        c = 0;
                        //return false;    // сразу выход с ошибкой !! дальше не грузит!!
                        ret = false;
                    }

                    //МАС проверять что есть с !!! или делать его =0
                    Match m1 = reg_mac.Match(str[i]);           //26F44A190100009B 2 группы
                    int num_gr1 = m1.Groups.Count;              // всегда=2 ??
                    //       if (Form1.fdbgpass == true) LOG.log_str("num_gr1 = " + num_gr1);  //
                    if (m1.Success)
                    {
                        int captureCtrMac;
                        for (int ctr = 1; ctr <= m1.Groups.Count - 1; ctr++)
                        {
                            //  LOG.log_str(String.Format("Group {0}:  {1}", ctr,m_name.Groups[ctr].Value));
                            int captureCtr = 0;
                            foreach (Capture capture in m1.Groups[ctr].Captures)  // заполнение
                            {
                                // вывод всех
                                //                if (Form1.fdbgpass == true) LOG.log_str(String.Format("#mac Capture {0}: {1}", captureCtr, capture.Value));
                                captureCtr += 1;
                            }
                            captureCtrMac = captureCtr;
                        }
                        String mac_cap = m1.Groups[1].Captures[0].Value;
                        mac_code = mac_cap;    ////mac! без пробела !!!!

                        //mac_code = m1.Value; //mac!  проверять на длину ??? и начальные 26 28 00 ??
                        //искл                       mac_code = m.Groups[1].Value;    //должно быть всегда ???
                        //          if (Form1.fdbgpass == true) LOG.log_str("m1 mac = " + m1.Value);  //mac! с пробелом !!!!

                        String sub = mac_code.Substring(0, 2);  // "00" для угла!??

                        if ((sub != "28") && (sub != "26") && (sub != "00")) //mac!  проверять на длину ??? и начальные 26 28 00 ??
                        {
                            LOG.log_str("load_all_passList: Коса: " + cyr_kosa + " , MAC = " + mac_code);  //
                            LOG.log_str("load_all_passList: Датчик не 26, 28, 00!! Начало = " + sub);  //mac!
                            ret = false;
                        }

                        if (mac_code.Length != 16)
                        {
                            LOG.log_str("load_all_passList: Коса: " + cyr_kosa + " , MAC = " + mac_code);  //
                            LOG.log_str("load_all_passList: Длина МАС адреса датчика не 16 байт! Длина = " + mac_code.Length);  //mac!
                            ret = false;
                        }
                    }
                    else LOG.log_str("load_all_passList: Нет МАС в начале строки паспорта косы: " + cyr_kosa);  //
                    //===================================
                    //коэф только ! берет не более 3!! так в reg_koeff
                    Match m_name = reg_koeff.Match(str[i]);           //
                    int num_gr_name = m_name.Groups.Count;   //групп две всегда !!!!!!!!
                    //       if (Form1.fdbgpass == true) LOG.log_str("num_gr_name = " + num_gr_name);  //
                    if (m_name.Success)                      //при  1 даст ???
                    {
                        //          if (Form1.fdbgpass == true) LOG.log_str("m_name = " + m_name.Value);  //

                        //int group1CtrCap = 0;      //кол.групп - всегда 2 ??
                        if (num_gr_name == 2)
                        {
                            int captureCtr1 = 0;     //кол в гр1
                            for (int ctr = 1; ctr <= m_name.Groups.Count - 1; ctr++)
                            {
                                //  LOG.log_str(String.Format("Group {0}:  {1}", ctr,m_name.Groups[ctr].Value));
                                int captureCtr = 0;
                                foreach (Capture capture in m_name.Groups[ctr].Captures)  // заполнение
                                {
                                    // вывод всех
                                    //                  if (Form1.fdbgpass == true) LOG.log_str(String.Format("#Koeff Capture {0}: {1}", captureCtr, capture.Value));
                                    captureCtr += 1;
                                }
                                captureCtr1 = captureCtr;
                            }
                            // captureCtr кол коэф   значение в 0 1 и тд
                            if (captureCtr1 < 2)  // менее 2 х коэф       // int num_koeff = 0;
                            {
                                LOG.log_str("load_all_passList: Коэф менее 2 (Match m_name error). Коса: " + cyr_kosa);  // gr = 3 и менее
                                LOG.log_str("load_all_passList: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                a = 1;
                                b = 0;
                                c = 0;
                                ret = false;
                            }
                            else if (captureCtr1 == 2)   // коэф  2
                            {
                                try
                                {
                                    a = Double.Parse(m_name.Groups[1].Captures[0].Value, nfi);     //
                                    b = Double.Parse(m_name.Groups[1].Captures[1].Value, nfi); //
                                }
                                catch (System.Exception)
                                {
                                    try
                                    {
                                        a = Double.Parse(m_name.Groups[1].Captures[0].Value.Replace('.', ','), nfi);  // для учета запятых или точек
                                        b = Double.Parse(m_name.Groups[1].Captures[1].Value.Replace('.', ','), nfi);
                                        //                  if (Form1.fdbgpass == true) LOG.log_str("Запятые заменены на точки 2 коэф");  //
                                    }
                                    catch (System.Exception)
                                    {
                                        a = 1;
                                        b = 0;
                                        c = 0;
                                        LOG.log_str("load_all_passList: 2 коэф. Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                        ret = false;
                                    }
                                }
                            }
                            else   // коэф более 2 - берет 3 !!!! остальные выбрасывает!!
                            {
                                try
                                {
                                    a = Double.Parse(m_name.Groups[1].Captures[0].Value, nfi);     //
                                    b = Double.Parse(m_name.Groups[1].Captures[1].Value, nfi);     //
                                    c = Double.Parse(m.Groups[1].Captures[0].Value, nfi);     //
                                }
                                catch (System.Exception)
                                {
                                    try
                                    {
                                        a = Double.Parse(m_name.Groups[1].Captures[0].Value.Replace('.', ','), nfi);  // для учета запятых или точек
                                        b = Double.Parse(m_name.Groups[1].Captures[1].Value.Replace('.', ','), nfi);
                                        c = Double.Parse(m_name.Groups[1].Captures[2].Value.Replace('.', ','), nfi);
                                        //                 if (Form1.fdbgpass == true) LOG.log_str("Запятые заменены на точки 3 коэф");  //
                                    }
                                    catch (System.Exception)
                                    {
                                        a = 1;
                                        b = 0;
                                        c = 0;
                                        LOG.log_str("load_all_passList: 3 коэф. Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                        ret = false;
                                    }
                                }
                            }
                        }
                        else // выход !!! с ошибкой gr>2
                        {
                            LOG.log_str("load_all_passList: Групп более 2 (Match error). Коса: " + cyr_kosa);  // gr > 2 !
                            LOG.log_str("load_all_passList: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                            a = 1;
                            b = 0;
                            c = 0;
                            ret = false;
                        }
                    }
                    else
                    {
                        LOG.log_str("load_all_passList: m_name: Нет коэф. в паспорте (Match m_name error) Коса: " + cyr_kosa);  //
                        LOG.log_str("load_all_passList: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                        a = 1;
                        b = 0;
                        c = 0;
                        ret = false;
                    }

                    //      if (Form1.fdbgpass == true) LOG.log_str("Конец строки");  //
                    //========================

                    ds18b20_param ds_tmp = new ds18b20_param();
                    ds_tmp.position = Param_list[Param_list.Count - 1].param.Count; //  кол.МАС в паспорте
                    ds_tmp.id = mac_code;
                    ds_tmp.a = a;
                    ds_tmp.b = b;
                    ds_tmp.c = c;                             //в ds18b20_param есть c !
                    ds_tmp.parent_index = Param_list.Count - 1; // номер в табл.коэф
                    Param_list[Param_list.Count - 1].param.Add(ds_tmp);

                    //kosa = ;
                    index++;

                    //Single a b c !!!!
                    //                              a   b  c   d  e   f
                    //             prog.kof_list.Add(new Koeff(mac_code, 0, 0, 0, c, b, a, -274, -3, kosa, index));


                } //else N для мас

            }  // for

            int pass_mac = Param_list.Count;
            LOG.log_str("load_all_passList: Загружены из passport.txt " + Param_list.Count + " косы");  //4 косы
            //polynom наверху

            for (int i = 0; i < strp.Count; i++)
            {
                string kosa = "";
                int index = 0;

                if (strp[i] == "") { continue; }
                //       if (strp[i] == "NEND") { break; }   // если короткое имя до 4 знаков!!
                if (strp[i][0] == 'N')
                {
                    // конец файла ставить в начале ! не попадает в паспорт!!
                    if (strp[i] == "NEND") { break; }   // если короткое имя до 4 знаков!!
                    ds18b20_plume tmp = new ds18b20_plume();
                    tmp.Name = strp[i];                          // имя косы
                    tmp.param = new std::vector<ds18b20_param>();      // будет отд коса с именем NEND !!! пустая
                    Param_list.Add(tmp);
                    cur_index++;
                    // для kof
                    kosa = strp[i];
                }
                else
                {
                    // 9 коэф
                    Regex reg = new Regex(@"([0-9a-fA-F]+)\s+([0-9\,\.\-\+eE]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)\s+([0-9\,\.\-\+Ee]+)");
                    //отдельно мас и коэф
                    Regex reg_mac = new Regex(@"([0-9a-fA-F]+)\s");  // c пробелом!  \A	Соответствие должно обнаруживаться в начале строки.
                    Regex reg_koeff = new Regex(@"(\s[0-9\,\.\-]+){9}");    // без пробела! в конце но обязательно в начале!! {9} шт
                    //

                    NumberFormatInfo nfi = new CultureInfo("en-US", false).NumberFormat;
                    nfi.NumberDecimalSeparator = ",";
                    nfi.NegativeSign = "-";
                    Double a, b, c, a2, b2, c2, a3, b3, c3;
                    //       Single a, b, c, a2, b2, c2, a3, b3, c3;
                    string mac_code = "";
                    //===============
                    Match m = reg.Match(str[i]);    // как было
                    int num_gr = m.Groups.Count;           //
                    //        if (Form1.fdbgpass == true) LOG.log_str("load_all_passList: num_gr = " + num_gr);  //
                    if (m.Success)                      //при 0 и 1 даст ошибку при 2 и более - вернет 2 !!
                    {
                        //        if (Form1.fdbgpass == true) LOG.log_str("load_all_passList: Строка паспорта m = " + m.Value);  //
                        mac_code = m.Groups[1].Value;    //должно быть всегда ???
                    }
                        //нет мас и 2-х коэф (если больше - возмет 2 !)
                    else
                    {
                        LOG.log_str("load_all_passList: Паспорт (Match m стандартная) с ошибкой в косе: " + cyr_kosa);  // при 9 1 коэф  - исключения не будет !!!
                        //mac_code = "0000000000000000";
                        LOG.log_str("load_all_passList: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                        a = 1;
                        b = 0;
                        c = 0;
                        //return false;    // сразу выход с ошибкой !! дальше не грузит!!
                        ret = false;
                    }

                    // string mac_code = m.Groups[1].Value;  // как было
                    //======================
                    // Match m_mac = reg_mac.Match(str[i]);    //отдельно мас

                    // проверять что есть с !!! или делать его =0
                    Match m_mac = reg_mac.Match(str[i]);           //26F44A190100009B 2 группы
                    int num_gr1 = m_mac.Groups.Count;              // всегда=2 ??
                    //         if (Form1.fdbgpass == true) LOG.log_str("load_all_passList: num_gr1 = " + num_gr1);  //
                    if (m_mac.Success)
                    {
                        int captureCtrMac;
                        for (int ctr = 1; ctr <= m_mac.Groups.Count - 1; ctr++)
                        {
                            //  LOG.log_str(String.Format("Group {0}:  {1}", ctr,m_name.Groups[ctr].Value));
                            int captureCtr = 0;
                            foreach (Capture capture in m_mac.Groups[ctr].Captures)  // заполнение
                            {
                                // вывод всех
                                //                   if (Form1.fdbg == true) LOG.log_str(String.Format("#mac Capture {0}: {1}", captureCtr, capture.Value));
                                captureCtr += 1;
                            }
                            captureCtrMac = captureCtr;
                        }
                        String mac_cap = m_mac.Groups[1].Captures[0].Value;
                        mac_code = mac_cap;    ////mac! без пробела !!!!

                        //mac_code = m1.Value; //mac!  проверять на длину ??? и начальные 26 28 00 ??
                        //искл                       mac_code = m.Groups[1].Value;    //должно быть всегда ???
                        //          if (Form1.fdbgpass == true) LOG.log_str("load_all_passList: m_mac = " + m_mac.Value);  //mac! с пробелом !!!!

                        String sub = mac_code.Substring(0, 2);

                        if ((sub != "28") && (sub != "26") && (sub != "00")) //mac!  проверять на длину ??? и начальные 26 28 00 ??
                        {
                            LOG.log_str("load_all_passList: Коса: " + cyr_kosa + " , MAC = " + mac_code);  //
                            LOG.log_str("load_all_passList: Датчик не 26, 28, 00!! Начало = " + sub);  //mac!
                            ret = false;
                        }

                        if (mac_code.Length != 16)
                        {
                            LOG.log_str("load_all_passList: Коса: " + cyr_kosa + " , MAC = " + mac_code);  //
                            LOG.log_str("load_all_passList: Длина МАС адреса датчика не 16 байт! Длина = " + mac_code.Length);  //mac!
                            ret = false;
                        }
                    }
                    else LOG.log_str("load_all_passList: Нет МАС в начале строки паспорта косы: " + cyr_kosa);  //

                    //=======================================
                    //коэф только ! берет  9!! так в reg_koeff
                    Match m_koeff = reg_koeff.Match(str[i]);     //отдельно коэф
                    int num_gr_koeff = m_koeff.Groups.Count;   //групп две всегда !!!!!!!!
                    //       if (Form1.fdbgpass == true) LOG.log_str("load_all_passList: num_gr_name = " + num_gr_koeff);  //
                    if (m_koeff.Success)                      //при  1 даст ???
                    {
                        //            if (Form1.fdbgpass == true) LOG.log_str("load_all_passList: m_name = " + m_koeff.Value);  //

                        if (num_gr_koeff == 2)      //кол.групп - всегда 2 ??
                        {
                            int captureCtr1 = 0;     //кол в гр1
                            for (int ctr = 1; ctr <= m_koeff.Groups.Count - 1; ctr++)
                            {
                                //  LOG.log_str(String.Format("load_all_passList: Group {0}:  {1}", ctr,m_koeff.Groups[ctr].Value));
                                int captureCtr = 0;
                                foreach (Capture capture in m_koeff.Groups[ctr].Captures)  // заполнение
                                {
                                    // вывод всех
                                    //                    if (Form1.fdbg == true) LOG.log_str(String.Format("#Koeff Capture {0}: {1}", captureCtr, capture.Value));
                                    captureCtr += 1;
                                }
                                captureCtr1 = captureCtr;
                            }
                            // captureCtr кол коэф   значение в 0 1 и тд
                            if (captureCtr1 != 9)  // не 9  коэф       //
                            {
                                LOG.log_str("load_all_passList: Коэф не равно 9 (Match m_name error). Коса: " + cyr_kosa);  // gr = 3 и менее
                                LOG.log_str("load_all_passList: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                a = 0; a2 = 0; a3 = 0;
                                b = 1; b2 = 1; b3 = 1;
                                c = 0; c2 = 0; c3 = 0;
                                ret = false;
                            }
                            else //   if (captureCtr1 == 9)   // коэф  9 и
                            {
                                try // T = a + b*t + c*t**2
                                {
                                    a = Double.Parse(m_koeff.Groups[1].Captures[0].Value, nfi);     //
                                    b = Double.Parse(m_koeff.Groups[1].Captures[1].Value, nfi); //
                                    c = Double.Parse(m_koeff.Groups[1].Captures[2].Value, nfi); //
                                    a2 = Double.Parse(m_koeff.Groups[1].Captures[3].Value, nfi);     //
                                    b2 = Double.Parse(m_koeff.Groups[1].Captures[4].Value, nfi); //
                                    c2 = Double.Parse(m_koeff.Groups[1].Captures[5].Value, nfi); //
                                    a3 = Double.Parse(m_koeff.Groups[1].Captures[6].Value, nfi);     //
                                    b3 = Double.Parse(m_koeff.Groups[1].Captures[7].Value, nfi); //
                                    c3 = Double.Parse(m_koeff.Groups[1].Captures[8].Value, nfi); //
                                }
                                catch (System.Exception)
                                {
                                    try
                                    {
                                        a = Double.Parse(m_koeff.Groups[1].Captures[0].Value.Replace('.', ','), nfi);  // для учета запятых или точек
                                        b = Double.Parse(m_koeff.Groups[1].Captures[1].Value.Replace('.', ','), nfi);
                                        c = Double.Parse(m_koeff.Groups[1].Captures[2].Value.Replace('.', ','), nfi);
                                        a2 = Double.Parse(m_koeff.Groups[1].Captures[3].Value.Replace('.', ','), nfi);  // для учета запятых или точек
                                        b2 = Double.Parse(m_koeff.Groups[1].Captures[4].Value.Replace('.', ','), nfi);
                                        c2 = Double.Parse(m_koeff.Groups[1].Captures[5].Value.Replace('.', ','), nfi);
                                        a3 = Double.Parse(m_koeff.Groups[1].Captures[6].Value.Replace('.', ','), nfi);  // для учета запятых или точек
                                        b3 = Double.Parse(m_koeff.Groups[1].Captures[7].Value.Replace('.', ','), nfi);
                                        c3 = Double.Parse(m_koeff.Groups[1].Captures[8].Value.Replace('.', ','), nfi);
                                        //                   if (Form1.fdbg == true) LOG.log_str("Запятые заменены на точки 9 коэф");  //
                                    }
                                    catch (System.Exception)
                                    {
                                        a = 0; a2 = 0; a3 = 0;
                                        b = 1; b2 = 1; b3 = 1;
                                        c = 0; c2 = 0; c3 = 0;
                                        LOG.log_str("load_all_passList: 2 коэф. Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                                        ret = false;
                                    }
                                }
                            }
                        }
                        else // выход !!! с ошибкой gr>2
                        {
                            LOG.log_str("load_all_passList: Групп более 2 (Match error). Коса: " + cyr_kosa);  // gr > 2 !
                            LOG.log_str("load_all_passList: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                            a = 0; a2 = 0; a3 = 0;
                            b = 1; b2 = 1; b3 = 1;
                            c = 0; c2 = 0; c3 = 0;
                            ret = false;
                        }
                    }
                    else
                    {
                        LOG.log_str("load_all_passList: m_name: Нет коэф. в паспорте (Match m_name error) Коса: " + cyr_kosa);  //
                        LOG.log_str("load_all_passList: Коэффициенты взяты по умолчанию!! МАС = " + mac_code);  //
                        a = 0; a2 = 0; a3 = 0;
                        b = 1; b2 = 1; b3 = 1;
                        c = 0; c2 = 0; c3 = 0;
                        ret = false;
                    }
                    //        if (Form1.fdbgpass == true) LOG.log_str("Конец строки");  //

                    ds18b20_param ds_tmp = new ds18b20_param();
                    ds_tmp.position = Param_list[Param_list.Count - 1].param.Count;
                    ds_tmp.id = mac_code;

                    ds_tmp.a = a;          // T = a + b*t + c*t**2
                    ds_tmp.b = b;        //b=1
                    ds_tmp.c = c;       // double для полинома

                    ds_tmp.a2 = a2;
                    ds_tmp.b2 = b2;
                    ds_tmp.c2 = c2;
                    ds_tmp.a3 = a3;
                    ds_tmp.b3 = b3;
                    ds_tmp.c3 = c3;
                    ds_tmp.parent_index = Param_list.Count - 1;
                    Param_list[Param_list.Count - 1].param.Add(ds_tmp);

                    index++;


                    cur_index++;
                }//else !=N для каждого мас

            }	//for polynom
            int poly_mac = Param_list.Count - pass_mac;
            LOG.log_str("load_all_passList: Загружены из polynom.txt " + poly_mac + " косы");  // 21
            LOG.log_str("load_all_passList: Загружены рабочих строк в файлах " + (cur_index - 1) + " ");  //в файлах 127
            LOG.log_str("load_all_passList: Загружены в параметры " + Param_list.Count + " кос");
            return ret;
            //  return true;
        }
//------------------------------------------------


    public static double compute_all(string rom, double raw_temp)
        {
            ds18b20_param param;
            param = search_param_all(rom);  // поиск в паспорте записи о МАС
            //if (Form1.chkDbg == true) { LOG.log_str("MAC: " + param.id); } //

            if (param.id != "NA")
            {
                if (raw_temp < -3)
                    return Math.Round(param.a + raw_temp * param.b + param.c * raw_temp * raw_temp, 4);
                if ((raw_temp >= -3) && (raw_temp <= 3))
                    return Math.Round(param.a2 + raw_temp * param.b2 + param.c2 * raw_temp * raw_temp, 4);
                if ((raw_temp > 3) && (raw_temp <= 30))
                    return Math.Round(param.a3 + raw_temp * param.b3 + param.c3 * raw_temp * raw_temp, 4);
                if (raw_temp > 30) { return raw_temp; }
            }
            else
            {
                LOG.log_str("MAC: " + rom + " не найден в паспорте!");  //
            }
            //return raw_temp;        //непересчитанная !!!
            return 0;           //просто 0 вместо пересчета !!!
        }

    public static double compute_all(int plume, string rom, double raw_temp)
        {
            ds18b20_param param;
            param = search_param_all(plume, rom);

            if (param.id != "NA")
            {
                if (raw_temp < -3)
                    return Math.Round(param.a + raw_temp * param.b + param.c * raw_temp * raw_temp, 4);
                if ((raw_temp >= -3) && (raw_temp <= 3))
                    return Math.Round(param.a2 + raw_temp * param.b2 + param.c2 * raw_temp * raw_temp, 4);
                if ((raw_temp > 3) && (raw_temp <= 30))
                    return Math.Round(param.a3 + raw_temp * param.b3 + param.c3 * raw_temp * raw_temp, 4);
                if (raw_temp > 30) { return raw_temp; }
            }
            else
            {
                LOG.log_str("MAC: " + rom + " не найден в паспорте!");  //
            }
            //return raw_temp;        //    //непересчитанная !!!
            return 0;           //просто 0 вместо пересчета !!!
        }
    public  static ds18b20_param search_param_all(string rom)
        {
            ds18b20_param empty = new ds18b20_param();
            empty.id = "NA";
            //position = ;        // заполняет при чтении Param_list[Param_list.Count - 1].param.Count;
            //parent_index = ;    // индекс в паспорте, заполняет ds_tmp.parent_index = Param_list.Count - 1; //
            empty.a = 0;
            empty.b = 1;
            empty.c = 0;               // T = a + b*t + c*t**2
            empty.a2 = 0;
            empty.b2 = 1;
            empty.c2 = 0;
            empty.a3 = 0;
            empty.b3 = 1;
            empty.c3 = 0;

            for (int i = 0; i < Param_list.Count; i++)   // 11 кос
            {
                //if (Form1.chkDbg == true) { LOG.log_str("Коса: " + Param_list[i].Name + " MAC search: " + rom); }
                //if (Form1.chkDbg == true) { LOG.log_str("MAC search: " + rom); }
                for (int j = 0; j < Param_list[i].param.Count; j++) //МАС в косе
                {
                    // if (Form1.chkDbg == true) { LOG.log_str("MAC: " + Param_list[i].param[j].id); }
                    if (String.Equals(Param_list[i].param[j].id, rom, StringComparison.OrdinalIgnoreCase))
                    {
                        //             if (Form1.Dbg == true) { LOG.log_str("MAC: " + rom + " search in kosa: " + Param_list[i].Name); }
                        return Param_list[i].param[j];
                    }
                }
            }
            return empty;       //
        }

    public static ds18b20_param search_param_all(int plume, string rom)
        {
            ds18b20_param empty = new ds18b20_param();
            empty.id = "NA";
            //position = ;        //
            //parent_index = ;    // индекс в паспорте
            empty.a = 0;
            empty.b = 1;
            empty.c = 0;               // T = a + b*t + c*t**2
            empty.a2 = 0;
            empty.b2 = 1;
            empty.c2 = 0;
            empty.a3 = 0;
            empty.b3 = 1;
            empty.c3 = 0;

            for (int j = 0; j < Param_list[plume].param.Count; j++)
            {
                if (String.Equals(Param_list[plume].param[j].id, rom, StringComparison.OrdinalIgnoreCase))
                {
                    //          if (Form1.Dbg == true) { LOG.log_str("MAC: " + rom + " search in kosa: " + Param_list[plume].Name); }
                    return Param_list[plume].param[j];
                }
            }
            return empty;       //
        }

        //
    public static int plume_index_all(string rom)
        {
            ds18b20_param param;
            param = search_param_all(rom);
            if (param.id != "NA")
            {
                return param.parent_index;
            }
            return -1;
        }


    public static string plume_name_all(string rom)
        {
            ds18b20_param param;
            param = search_param_all(rom);
            if (param.id != "NA")
            {
                return Param_list[param.parent_index].Name;  // имя косы
            }
            return "Нет Данных";
        }

        // изменено   check_id(string rom) !!!

        //-----------------------------------------------------------------------




    }  // passport
    */
