#ifndef LOGGER_PASSPORT_H
#define LOGGER_PASSPORT_H     1

#include <string>
#include <istream>
#include <vector>
#include <inttypes.h>

/**
 * Initialize logger password directory
 * @param passwords_path path to the catalog with password files
 * @param verbosity if 1, 2 or 3 print out to the stderr errors parsing declarations
 * @return descriptor of the passwords to be passed to the parsePacketsToSQLClause()
 */
void* initLoggerPasswords(
        const std::string &passwords_path,
        const int verbosity = 0
);

/**
 * Destroy and free logger properties
 * @param env descriptor
 */
void doneLoggerPasswords(void *env);

// типы коэфф. для загрузок по одному в основной !! Std,Poly,Akon,Inklinom,Angl
// для passport.txt polynom.txt akon inclinom.txt
typedef enum  {
    Std,        // passport.txt ds18b20
    Poly,       // polynom.txt
    Akon,       // akon akon_adapter.txt (akon_polynom.txt , akon_int.txt)
    Inklinom,   // для старого для passport.txt polynom.txt akon inclinom.txt
    Angl,       // angl_pas.txt - для нового
    all         // читать все!!!
} KoeffType;

typedef float Single;

/*********************** коэф класс для датчиков темп *************************
*        Class name : класс коэф. для датчиков темп. один диапазон? до 5 степени? (6 коэф)
 *        если диапазонов несколько - отдельные строки!
*        Purpose :
 *        сделать double  !!!!!!!!
******************************************************************************/
class Koeff {
public:
    std::string mac;
    int index;
    std::string kosa;
    Single a, b, c, d, e, f; //std 2 koef e f
    Single sTemp;    //
    Single eTemp;    //

    // вычисл. реальной темп. с учетом коэф. паспорта
    Single compute(Single x);
    // проверка попадания в разрешенный диапазон
    bool check(Single x);

    Koeff(std::string nmac, Single na, Single nb, Single nc, Single nd, Single ne, Single nf, Single nst, Single nen,
          std::string nkosa, int nIndex);
    Koeff();
};

/******************* добавить коэф.для koeff_list_angl **************************
 *        Purpose : добавить коэф.для темп.
 * вычисление с учетом чувств. и нач. сдвига градусы (или 16 бит?? count)
  //# MAC узла  	kx0off, ky0off, kz0off, kxs, kys, kzs
  //N1_
   00ED521901000014 	0   0   0   0.02197265625	0.02197265625 0.02197265625
 * 26ED521901000014 	1   0	0   0  0  0
 * double anglxyz = value * 0.02197265625; //COUNT_GRAD;
 * mg/count 0,244140625
 * koeff_list_angl новый лист для коэф.углов  angl_pas.txt
 ******************************************************************************/
class KoeffXYZ
{
public:
    std::string mac;
    int index;
    std::string kosa;
    Single kx0off, ky0off, kz0off, kxs, kys, kzs; // по 2 koef for angl
    KoeffXYZ(std::string nmac, Single x0off, Single y0off, Single z0off, Single xs, Single ys, Single zs,
        std::string nkosa, int nIndex);
};

class Angle
{
public:
    std::string mac;
    std::string kosa;
    int index;                  // индекс в файле паспорта, в косе?
    std::string time;           // время каждого замера даже при повторах
    Single temp;                // без коэфф. !!!!
    double newTemp;             // пересчитанная с коэфф.
    std::string versionK485;    // версия
    uint8_t ID_Slave;           // ид слэйва
    double anglX;               // угол в градусах (без офсетов и коэффициентов темп.??)
    uint16_t valX;              // =valueX угол первичный без сдвига 2 бит!? count_X уже со сдвигом
    double count_X;             // cчет по Х
    bool znak_X;                // знак !!!!
    double cosanglX;            // нормальный угол с осью
    double anglY;
    uint16_t  valY;
    double count_Y;             // cчет по
    bool znak_Y;                // знак !!!!
    double cosanglY;            // нормальный угол с осью
    double anglZ;
    uint16_t valZ;
    double count_Z;             // cчет по
    bool znak_Z;                // знак !!!!
    double cosanglZ;            // нормальный угол с осью
    double gg;                  // должен быть 1 !!
    Single vBat;
    double gr;                  // должен быть 1 !! длина вектора относит. 0-1
    double yz;                  // ТОЛЬКО Y Z !!
    double yz_kv;               // ТОЛЬКО Y Z !!
    double len_vec;             // длина вектора абсолютютная xyz 0-4096 по замеру
    int quadrant;               // квадрант для углов
    double shift;               // сдвиг в плане на 1 shift = yz / Convert.ToDouble(4096);  // относительная от шкалы в 4096,
    double shift_Y;             // сдвиг по оси без коэф!!!!
    double shift_Z;             // сдвиг по оси
    double anglSig_X;           // 3 сигмы для основного замера - не дельт
    double anglSig_Y;
    double anglSig_Z;
    double koef_shift;

    Angle();
    // mac не добавлен?! мас идет из листа ИД в порядке ИД!! а не паспорта
    void tempAdd(int lsb, int msb, std::string ds);
    // класса Angl - заполнение счета в двойной со знаком
    void countsAdd(uint16_t &valueX, uint16_t &valueY, uint16_t &valueZ);
    //функции для  Angl !!! расчет gr cosX не по среднему углу, а по средним count !!!!!
    void setAngl();

    void anglAdd(uint16_t valueX, uint16_t valueY, uint16_t valueZ);
    bool CheckMac(std::vector<Koeff> &koef_data);
    bool computeTemp(std::vector<Koeff> &koef_data);
    bool computeTempPol(std::vector<Koeff> &koef_data);
    void pwrAdd(int lsb);
    //добавить замер, лист коэф. koeff_list_angl для температуры! только?
    bool recValAdd(uint16_t val[], KoeffType type, std::vector<Koeff> koef_data);
private:
    double computeAngl(uint16_t value);
};

// параметры устройства
typedef struct
{
    int num;                 //номер в году НЕ ПИШЕТ пока
    std::string devAddr;          //
    std::string devAppSKey;       //
    std::string devNwkSKey;       //

    std::string devEUI;           //
    std::string devAppKey;        //

    std::string devHash;         //
    std::string devAESKey;
    std::string devDescr;         //описание
} dev_param;

//структура для года и парам.
typedef struct
{
    std::string Year;         //год
    int n_year;         //
    std::vector<dev_param> param;     //devaddr и парам для него
    std::string Descr;         //
} dev_year;

class prog {
public:
    bool fdebug;
    bool f_ds2438;
    std::vector<Koeff> kof_list;     // используется для темп.
    std::vector<KoeffXYZ> koeff_list_angl;   //добавлено для нового инкл angl
    std::vector<Angle> angl0id_list;     //лист 0 замеров по ID static
    // вычисление отрацат.темп. для ds2438
    static Single minusTempCompute(int msb, int lsb);
};

class passport
{
public:
    std::string getStringByte(std::string filename, int mode);
    int razbor(std::string str, std::vector<std::string> sub_list);
    std::string unComment(std::string str);
    std::vector<std::string> getString(std::istream &instream);
    std::vector<std::string> getString(std::string filename);

    std::vector<dev_year> dev_param_list;
    // LORA
    int cyr_year;  // текущий год.
    int num_cyr_year;  // кол.в тек.году

    //НЕ ИСПОЛЬЗУЕТСЯ создать и заполнить структуру для года и параметров
    static void getEmptyStruct(int cyr_year);
    int getNumCyrYear();
    int getCyrYear();
    bool loadDev(std::istream &mstream);

    bool loadDevStr(std::vector<std::string> str);
    bool loadDev(std::string filename);
    // ПЕРЕМЕННЫЕ И СТРУКТУРЫ для температуры
    // параметры коэфф. (косы ) полином- 9 коэф в одну строку для мас
};

class ds18b20_param     //надо создавать для каждого!
{
    std::string id;           //МАС
    double a;         //
    double b;        //
    int position;        //
    int parent_index;    //
    double c;   //  ds_tmp.c = c1;   //доб. для полинома

    double a2;   //ds_tmp.a2 = a2;
    double b2;   //ds_tmp.b2 = b2;
    double c2;   //ds_tmp.c2 = c2;
    double a3;   //ds_tmp.a3 = a3;
    double b3;   //ds_tmp.b3 = b3;
    double c3;   //ds_tmp.c3 = c3;

    ds18b20_param();
};

class ds18b20_plume
{
    std::string Name;         //имя косы
    std::vector<ds18b20_param> param;

    ds18b20_plume();

    // лист паспорта сам статик - обьекты нет!! надо новый!!
    std::vector<ds18b20_plume> Param_list;
    bool load(std::istream mstream, int type);
    bool load(std::string &filename, int type);
    //добавить из файла без обнуления - типы 0 1 2
    bool loadAdd(std::string filename, int type);

    // подпрограмма чтения коэф. 3 шт !! std poly и angl только !! читает по одному !!!! по типу
    // добавлен тип новый инкл Angl
    //  чистить параметры!!! если тип не все All=5 ??
    bool load_koef(std::vector<std::string> str, int type);
    // без стирания Param_list !! и остальных!! для добавления просто!!
    // для паспорта aх b и полинома коэф a b наоборот!!!!
    bool load_koefAdd(std::vector<std::string> str, int type);
    ds18b20_param search_param(std::string rom);
    ds18b20_param search_param_poly(std::string rom);
    ds18b20_param search_param(int plume, std::string rom);
    ds18b20_param search_param_poly(int plume, std::string rom);
    bool check_id(std::string rom);
    double compute(std::string rom, double raw_temp);
    double compute_poly(std::string rom, double raw_temp);
    double compute(int plume, std::string rom, double raw_temp);

    double compute_poly(int plume, std::string rom, double raw_temp);
    std::string plume_name(std::string rom);
    int plume_index(std::string rom);
    //--------------------- работа со всеми паспортами -------------------------------------------------
    //через load() и loadAdd() из текста файлов в основной!!
    //последовательное чтение трех последовательно!! файлов с темп. коэф
    bool load_all_passNew();

    //2 листа для чтения std poly сразу - инклинометров и акона нет!!!
    // коэфф всего 3!!!! - как лоад но последовательно 2 типа (файла)
    bool load_all_passList(std::vector<std::string> str, std::vector<std::string> strp);


    double compute_all(std::string rom, double raw_temp);

    double compute_all(int plume, std::string rom, double raw_temp);
    ds18b20_param search_param_all(std::string rom);
    ds18b20_param search_param_all(int plume, std::string rom);

    int plume_index_all(std::string rom);
    std::string plume_name_all(std::string rom);
};


#endif
