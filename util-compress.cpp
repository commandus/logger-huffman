#include <iostream>

#include "util-compress.h"

// #define DEBUG_PRINT 1

// сжатие по Хаффману!
// буфер 2 памяти, в нем дельты с шапкой замера!!!
// в начале шапки замера used два байта -не жмутся! Общая длина дельт 10+2*cnt_mac или 10+cnt_mac
uint16_t compressLogger(
    const char *buff_in,
    char *buff_out,
    uint16_t len
)
{
    // bufferRes2 - входной буфер дельт
    // len = len_zip_rec = cnt_mac * len_rec + TITLE_COMPRESS; 4 * 2 +10
    uint16_t sym_sim_bit = 0;   // подсчет бит в выходном сообщении
    uint16_t i;                 // индекс во входном буфере?
    uint16_t bufpos = 0;        // индекс в выходном буфере! надо +10???
    uint16_t bitbuf = 0;        // длина сформ. символа + предыдущие - битовый буфер накопления
    uint8_t symbol;             // текущий символ для вставки
    uint8_t symbol_real = 0;    // реальный символ после префикса
    uint8_t priznak = 0;        // префикс
    uint8_t symblen = 0;        // длина
    uint8_t symbcode = 0;       // код -текущие из массивов
    uint8_t bits = 16;          // длина - битовый буфер накопления

    uint8_t tab_mask[] = {0x00, 0x01, 0x03, 0x07, 0xF, 0x1F, 0x3F, 0x7F, 0xFF}; // [9] от 0 !! кол бит 1 2 3 4 5 6 7 8

    // 2 первых байта это номер замера! в шапке замера - не сжимать - сразу писать в выходной буфер
    // в выходном буфере для длины!!!! надо оставить 2 байта в начале! + 8 байт шапки общей!! те 10 байт

    // дельта лежит в bufferRes2 !!!! 2 байта не сжимать - перезапись
    // lsb читать 2 байта номера замера из буф 1 - used lsb msb
    buff_out[bufpos] = buff_in[0];      //lsb used не жмет buff_out ссылка на внеш. буфер!
    bufpos++;
    buff_out[bufpos] = buff_in[1];       //msb used не жмет
    bufpos++;
    // еще delta_sek !!!! она жмется!!!

    // шапка замера                                                              || тело замера
    // ..00..00..         00..     00.. 00..00..00..00.. 00..00.        .00..00.. ||   00..00.   .00..00    ..00..00..  -18 байт
    // used           d_sek          шапка (побайтно)                            ||    замеры (пословно)
    // 0 =1 бит!        16 байт = 16 бит = 2 байта!!

    // для каждого остального символа
#ifdef DEBUG_PRINT
    std::cerr << "compress: Начало кода bufpos = " << (int) bufpos << ", циклов = " << (int) (len - 2) << std::endl;
#endif
    for (i = 0; i < (len - 2); i++) {
        symbol = buff_in[i + 2];     //чтение символа, текущего -buff_in ссылка на внеш. входной буфер!
        // как байт! в диапазоне +-4, менее 4 это 0-4 ((symbol >= 0) && (symbol <= 4))
        if ((symbol <= 4) || (symbol >= 0xFC))   //для 0 1 2 3 4  FF FE FD FC pref - в таблице
        {
            // if (fdbg) LOG.log_str(String.Format("compress: диапазон Байт из буфера symbol={0:X2}", symbol));
            // вычисление индекса в таблице из отриц.числа 8 - (7 - 6 - 5)
            if (symbol > 0xFB) {
                symbol = (uint8_t) (8 - (uint8_t) (0xFF - symbol));
            } // 0xFF-0xFC= 0x03 2 1 0 (FF-FB=4)
            // private uint8_t[] symblenHafCod  = new uint8_t[]{ 1, 4, 5, 6 , 7, 7, 5, 5, 4, 2 };
            // private uint8_t[] hafCanonCod = new uint8_t[]{ 0x01,0x02,0x01,0x01,0x00,0x01,0x02,0x03,0x03,0x01};
            // индекс=symbol      symbHafCod hafCanonCod    symblenHafCod
            // 0x00  =0                        0x01               1
            // 0x01  =1                        0x02               4
            // 0x02  =2                        0x01               5
            // 0x03  =3                        0x01               6
            // 0x04  =4                        0x00               7
            // 0xFB  8-(0xFF-0xFB)=8-4=4   нет! не >=0xFB а просто > !!!
            // 0xFC  8-(0xFF-0xFC)=8-3=5       0x01               7
            // 0xFD  8-(0xFF-0xFD)=8-2=6       0x02               5
            // 0xFE  8-(0xFF-0xFE)=8-1=7       0x03               5
            // 0xFF  8-(0xFF-0xFF)=8-0=8       0x03               4
            // 0x08  9                         0x01               2
            // if (fdbg) LOG.log_str(String.Format("compress: Индекс в таблице длин symbol={0:X2}", symbol));
        } else {   //признак=1 след.байт без изм. (символ = индексу в табл. признак дает)
            // if (fdbg) LOG.log_str(String.Format("compress: Байт из буфера symbol={0:X2}", symbol));
            symbol_real = symbol;    //уже реальный байт =0x08
            symbol = 9;            //как индекс!? в symbHafCod, было =9
            priznak = 1;  //его длина = 2 бита
            // if (fdbg) LOG.log_str(String.Format("compress: priznak={0:D} symbol = 1", priznak));
        }

        // если symbol = 9 - расширение! priznak = 1;
        uint8_t baseHaf[] = {0x3f, 1, 1, 2, 2, 1, 1, 0}; // [0..7] ?=0x3f для декода
        // private uint8_t[] offsHaf = new uint8_t[]    { 0x3f, 1, 1, 0x3f, 2, 1, 1, 0 }; //[0..7] для декода
        // private uint8_t[] symbHaf = new uint8_t[]    { 0x00, 0x08, 0x01, 0xff, 0x02, 0xfd, 0xfe, 0x03, 0x04, 0xfc }; //[0..9]
        // private uint8_t[] symblenHaf = new uint8_t[] { 0x01, 0x02, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x07, 0x07 }; //для код [0..9]
        // private uint8_t[] hafCanon = new uint8_t[]   { 0x01, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0x01, 0x00, 0x01};
        // hafCanon = {0b1,0b01,0b0010,0b0011,0b00001,0b00010,0x00011,0b000001,0b0000000,0x0000001};  //биты []
        // ДЛЯ КОДА
        // замена tabl_haf = hafCanon, tabl_len=symblenHaf,tabl_haf_symbol = symbHaf
        uint8_t symblenHafCod[] = {1, 4, 5, 6, 7, 7, 5, 5, 4, 2};
        uint8_t hafCanonCod[] = {0x01, 0x02, 0x01, 0x01, 0x00, 0x01, 0x02, 0x03, 0x03, 0x01};
        // по символу найти индекс в symbHaf или для кода symbHafCod сразу и symblenHafCod
        //  int symbolInd = symbHaf.Length;  //10
        //  for (int j = 0; j < symbHaf.Length; j++)
        ///  {
        //     if (symbHaf[j] == symbol) { symbolInd = j; break; } //0-9
        ///  }
        // подпрограмма записи битов и префикса перед байтом тоже!!
        // symblen = symblenHaf[symbolInd];   //0x02  для 0 =0x01 символ = индекс в табл. 0-8  9
        // symbcode = hafCanon[symbolInd];    //для 0 =0x01 если =9 - надо писать реальный и признак 2 бита
        // без symbolInd
        symblen = symblenHafCod[symbol];
        symbcode = hafCanonCod[symbol];
        // symbcode = tabl_haf_revers[symbol];  //=0x02 для 0 =0x01 _revers биты наоборот!! от ст. к мл.

        sym_sim_bit = (uint16_t) (sym_sim_bit + symblen);   //накопление длины выходного сообщения в битах !

        // if (fdbg) LOG.log_str(String.Format("compress: i={0:D} symblen={1:X2} symbcode={2:X2} symbol = buff_in[i + 2]={3:X2}", i, symblen, symbcode, buff_in[i + 2]));
        // if (fdbg) LOG.log_str(String.Format("compress: i={0:D} bits={1:D} sym_sim_bit={2:D}", i, bits, sym_sim_bit));

        // длина символа мин.=1 ( напр. 5 и 1  или ) symblen <= 8 !!!!!!!!!!
        // еще есть место в битбуфере!
        if (symblen <= bits)     //в начале =16!! мин = 1 !! 0x01 <= 15 <= оставшееся кол.бит в битбуфере
        {
            // symblen минимум =1 но bits может быть 0!!
            bitbuf <<= symblen;        //0х02 для префикса как инт ! 0111 1111 1111 1111 << 1 
            bitbuf |= (uint16_t) symbcode;   //0х02 для префикса  //0x0000 | 0x01
            bits = (uint8_t) (bits - symblen);     //(bits(15) - symblen) - не  может быть минус! только 0 !!

            // if (fdbg) LOG.log_str(String.Format("compress: symblen <= bits i={0:D} bits=bits - symblen={1:D}", i, bits));
            // если последний и 0 надо сохранять!!!  остатка не будет!!
            if ((bits == 0) && (i == (len - 2 - 1)))  //последний!!
            {
                buff_out[bufpos] = (uint8_t) (bitbuf >> 8);
                bufpos++;   //msb
                buff_out[bufpos] = (uint8_t) bitbuf;
                bufpos++;   //lsb
            }
        }
        //нет места в битбуфере!
        else {
            // symblen > bits, symblen (до 7) > bits (те bits<7) больше чем осташееся кол.бит в битбуфере или 0!! если предыдущий проход
            // дал 0 в bits (16 проход 1<= 0) при кратных длинах 1 3 5  15 заполнен бит буфер полностью !
            // if (fdbg) LOG.log_str(String.Format("compress: symblen > bits i={0:D} symblen{1:X2} bits={2:D}", i, symblen, bits));
            // битбуфер растет от мл. к ст.
            // ЕСЛИ УЖЕ bits=0 НИЧЕГО НЕ ДОБАВИТ!!!
            if (bits == 0)        // СНАЧАЛО СОХРАНИТЬ потом дополнить!!!!
            {
                // if (fdbg) LOG.log_str(String.Format("compress: ost i={0:D} bitbuf={1:X4} bits == 0!!", i, bitbuf));
            }

            // сдвиг на 0 оставит прежним
            bitbuf <<= bits;      //ост.бит bits <7 сдвиг по оставшимся (может быть уже 0 на 16 символе, 6 или 4 ?!!)
            bitbuf |= (uint16_t) ((symbcode >> (symblen - bits)) & tab_mask[bits]);

            // bitbuf |= (uint16_t)(symbcode & tab_mask[bits]);    //bits=1 tab_mask[1]=1, (tab_mask[0] = 0;)
            // это в старшие байт!? нужно только старшие биты ??
            // symbcode_s = symbcode >>(symblen - bits);  // в след буфер   код 2 = 00110 длина 5, будет 0 0110 ост 4
            // if (fdbg) LOG.log_str(String.Format("compress: ost i={0:D} bitbuf={1:X4} bits={2:D}", i, bitbuf, bits));
            //          if (fdbg) LOG.log_str(String.Format("compress: ost i={0:D} tab_mask[bits]={1:X2} symbcode={2:X2}", i, tab_mask[bits], symbcode));
            // было

            // сдесь сохранять побайтно! первые байты в ст.байте!!?? msb lsb - разбор наоборот! со ст.бит
            // *(ptr_out + bufpos) = (uint8_t)bitbuf;

            // lsb (надо наоборот ????? тк в ст.бите начало!)
            buff_out[bufpos] = (uint8_t) (bitbuf >> 8);    //msb
            bufpos++;
            buff_out[bufpos] = (uint8_t) bitbuf;    //lsb
            bufpos++;
            // if (fdbg) LOG.log_str(String.Format("compress: out i={0:D} symb msb={1:X2} symb lsb={2:X2}", i, buff_out[bufpos - 1], buff_out[bufpos-2]));

            bitbuf = 0;     //снова пустой битбуфер - 2 байта выведены bits=16; ??
            // symblen до 7   bits до 16 ????  bits <7
            // добавить к новому остаток от  дл.=5 бит symbcode=0x06  прямой 06= 000 00110
            //           if (fdbg) LOG.log_str(String.Format("compress: new bitbuf = 0 i={0:D} symblen={1:X2} bits={2:D}", i, symblen, bits));
            //                       000 0011 0  >  1 ??
            //  bitbuf |= (uint16_t)(symbcode >> bits); //bits =1  0 0110 > 1
            // мл. не сдвигать! а только по маске tab_mask[symblen - bits]  - остаток для прямого
            // при bits=0 сохранит symbcode
            bitbuf |= (uint16_t) (symbcode & tab_mask[symblen - bits]);

            bits = (uint8_t) (16 - (uint8_t) (symblen - bits)); //  новая длина

            // if (fdbg) LOG.log_str(String.Format("compress: new i={0:D} bitbuf={1:X4} symbcode={2:X2} bits={3:D}", i, bitbuf, symbcode, bits));

            // if (fdbg) LOG.log_str(String.Format("compress: new i={0:D} bits={1:D}", i, bits));

            // подпрограмма записи байта (не сжатого)
            if (priznak == 1)         //+  подпрограмма записи байта- битов
            {
                priznak = 0;             // обнулить признак
                symblen = 8;             // длина байта
                symbcode = symbol_real;  //

                sym_sim_bit = (uint16_t) (sym_sim_bit + symblen);

                if (symblen <= bits)       //длина меньше остатка битбуфера
                {
                    bitbuf <<= symblen;    // 8 перед этим 2     05 0 0 0 0 0 02=0x0C дл.=5

                    bits = (uint8_t) (bits - symblen);   //еще не выводить
                    // if (fdbg) LOG.log_str(String.Format("compress: priz=1 out i={0:D} symbol={1:X2}",i, symbcode));
                } else     //длина больше остатка битбуфера -заполняется от lsb к msb
                {
                    bitbuf <<= bits; //
                    bitbuf |= (uint16_t) ((symbcode >> (symblen - bits)) & tab_mask[bits]);  // старшие биты!
                    // сдесь сохранять побайтно!
                    buff_out[bufpos] = (uint8_t) (bitbuf >> 8);
                    bufpos++;
                    buff_out[bufpos] = (uint8_t) bitbuf;
                    bufpos++;
                    // if (fdbg) LOG.log_str(String.Format("compress: priz=1 out i={0:D} symbm={1:X2} symbl={2:X2} ", i, buff_out[bufpos - 1], buff_out[bufpos - 2]));

                    bitbuf = 0;
                    bitbuf |= (uint16_t) (symbcode & tab_mask[symblen - bits]);
                    //               /bits = 15 - (symblen - bits);    // 4     остаток бит
                    bits = (uint8_t) (16 - (uint8_t) (symblen - bits)); //  новая длина
                }       //else
            }  // if priznak=1 целый байт
            // else  // priznak=0

            // if (fdbg) LOG.log_str("compress: bufpos=" + bufpos + " bitbuf=" + String.Format("{0:X4}", bitbuf));
            // в цикле - текущая проверка
        } // for для i < (len - 2);
        // if (fdbg) LOG.log_str(String.Format("compress: bits={0:D} sym_sim_bit={1:D} bytes={2:D}", bits, sym_sim_bit, sym_sim_bit / 8));   //
        // if (fdbg) LOG.log_str(String.Format("compress: bitbuf={0:X4}", bitbuf));   //
#ifdef DEBUG_PRINT
        std::cerr << "compress: ostatok sym_sim_bit%16 ost = " << (int) sym_sim_bit % 16 << " bits = " << (int) bits << std::endl;
#endif
        if ((bits > 0) && (bits != 16)) { //это не заполненный остаток!! напр. 15 - исп.только 1 бит, 0 весь занят! Сохранен!!, 16 свободен
            // if (fdbg) LOG.log_str(String.Format("compress: ost > 0 последний не записан (или не весь!) symblen={0:X2} symbcode={1:X2}", symblen, symbcode));   //
#ifdef DEBUG_PRINT
            std::cerr << "compress: ost > 0 последний не записан (или не весь!)" << std::endl;
#endif
            for (i = 0; i < bits; i++) { //bits =15
                bitbuf = (uint16_t) (bitbuf << 1);
                bitbuf |= (uint16_t) 1;
                // if (fdbg) LOG.log_str(String.Format("compress: bits != 16 j={0:D} bitbuf={1:X4}", j, bitbuf));
            }

            buff_out[bufpos] = (uint8_t) (bitbuf >> 8);        //msb всегда
            bufpos++;
#ifdef DEBUG_PRINT
            std::cerr << "compress: msb bits != 16 i = " << (int) i << " symbm = " << (int) buff_out[bufpos - 1]
                      << std::endl;
#endif

            sym_sim_bit = (uint16_t) (sym_sim_bit + bits - 8); //33+15=48 -8=40

            if ((16 - bits) > 8)     // (16- bits) использовано
            {
                buff_out[bufpos] = (uint8_t) bitbuf;
                bufpos++;
#ifdef DEBUG_PRINT
                std::cerr << "compress: lsb bits != 16 i = " << (int) i << " symbol = " << (int) buff_out[bufpos - 1]
                          << std::endl;
#endif
                sym_sim_bit = (uint16_t) (sym_sim_bit + 8); //40+8=48
            }
        }

        sym_sim_bit = (uint16_t) (sym_sim_bit + 16);
#ifdef DEBUG_PRINT
        std::cerr << "compress: out bufpos = " << (int) bufpos << " sym_sim_bit = "
                  << (int) sym_sim_bit << " bytes = " << (sym_sim_bit / 8) << std::endl;;
#endif
        // ЕСЛИ НЕ ПОЛНЫЙ БАЙТ БУДЕТ БОЛЬШЕ !! раскод. по длине датчиков!!!
        return bufpos;        // от 0 ! - количество записанных байт в выходном буфере ( +1 ???) или нужно бит!??
    }
}

// для темп. собирать из 2-х байт слово, hafman нужно без!? те -1
// return битовую длину 13-2 или 0
// len - длина тела!! cnt_mac передается внешнее
/*
static uint8_t loggerBitlenBody(
    const uint8_t *buff_in,
    int len,
    int cnt_mac
)
{
    uint8_t i;    // до 128 ??!
    uint16_t bitsumW = 0;  //до 12 bits
    uint16_t wordT = 0;  //
    uint8_t bitlenW = 0;  //

    // в записи по 4 байта для в4! после дельты 2 байта на темп -lsb msb без маски
    // тк нет сохранения можно i как индекс в buff_in - тогда проще!
    int TITLE_COMPRESS = 10;
    for (i = 0; i < cnt_mac; i++) { // в буфере 2 записан 0 замер
        wordT = 0;  //TITLE_COMPRESS=10
        // определить длину битовую без шапки ТОЖЕ ДО 8 МОЖНО ОБЩУЮ!!??? для байта брать со 2!
        // после дельты4 всегда еще 2 байта!!
        wordT = buff_in[i * 2 + TITLE_COMPRESS + 0]; //continue; } //чет lsb 10 12 ..
        wordT |= (uint16_t) (buff_in[i * 2 + TITLE_COMPRESS + 1] << 8); //} //msb нечет 11 13 15 ..
        // if (fdbg) LOG.log_str(String.Format("loggerBitlenBody: wordT = 0x{0:X4}", wordT));
        // wordT = (uint16_t)(wordT & 0x0FFF);  //убрать 4 бита ст.??
        if (wordT > 0x800) { // если отрицательное брать доп.код - он плюс!
            wordT = (uint16_t) (~wordT);
            wordT++;
            // if (fdbg) LOG.log_str(String.Format("loggerBitlenBody: minus wordT = 0x{0:X4}", wordT));
        }
        bitsumW |= wordT;     //если добавлят все 1 сразу - маска будет и длиной!
        // if (fdbg) LOG.log_str(String.Format("loggerBitlenBody: bitsumW = 0x{0:X4}", bitsumW));
    }
    if (bitsumW == 0) {    // все 0 !!! не делать for !!!!
        //  buff_out[bufpos] = 0x80;    //остатка не будет !!???
        //  bufpos++;
#ifdef DEBUG_PRINT
        std::cerr << "loggerBitlenBody: Тело все 0!!!" << std::endl;
#endif
        //  return bufpos;    //
        return 0;    //битовая длина =0 !! остальные 13-2
    } else {   //не все биты = 0
        //12 бит это уже знак в температуре!!
        bitlenW = 0;  //длина битовая - писать в шапку пакета!? в статус (хватит 4 бита до 16)

        // поиск ст.бита макс кол. - длина данных включая минус! len = 12; ранее len = 8;
        for (i = 12; i > 0; i--)  //12-1 для байта максимум 7 тк сначала сдвиг? для 1111 1111 i= 0 надо 8
        {
            //в бите =1 - макс.длина с лево - ст.бит  0000 0100 0000  при i=5  и 12-5=7
            if (((bitsumW << (12 - i)) & 0x800) > 0) //бит есть! или сразу +1 и от 1 до 12
            {
                bitlenW = i;   // 12-1 с маской на 1 бит длинее (0 1 и 12 быть не должно!!)
                bitlenW = (uint8_t) (bitlenW + 1);  // длина знака с маской минус!  13-2 !! 0 и 1 нет!
                break;
            }
        }
        // if (bitlenW == 0) { }      //если if (bitsumW == 0) НЕ БУДЕТ!!  не найдено!!! для 0
        // >=1 ? если есть хоть один! брать 6 бит макс. число и бит знака! всего 7 бит
        // if (bitlenW < 9) //это  1 2 3 4   5 6 7 (те число (дельта) менее 127 без знака!) можно сжимать для всех одна
        /// {
        // len_rec = 1;  // глобальная!??
        ///}
        // if (bitlenW == 13) { }      //все 12 бит! и еще и маска - не может!? 12 уже с минус
        // при 0х0001 бит длина =2
        // if (bitlenW == 1)  { return 0;   }  //только маска! все 0! писать спец символ!?? типа 80 это как минус 0 - все 0
    }
#ifdef DEBUG_PRINT
    std::cerr << "loggerBitlenBody: Бит длина тело bitlenW = " << (int) bitlenW << std::endl;
#endif
    return bitlenW;  // битовая длина 13-2 или 0
    // далее компрессия маску брать из табл. по bitlen - 1
}
*/

/// декодирование Хафмана - выходной для декодера bufferPack2 
//для Хафмана входной буфер это буфер -массив принятого тела замера!!!
// выходной для декодера bufferPack2 (+ 10? !!) на выходе дельта
// входной буфер это массив тела пакета
// len = общая длина = compress_len - 8, на выходе дельта
uint16_t decompressLogger(
    const char *buff_in,
    char *buff_out,
    uint16_t lenComp
)
{
    uint8_t j;
    uint16_t bufpos = 0;		//индекс во входном буфере!
    uint16_t bufpos_out = 0;		//индекс в выходном буфере!

    uint16_t bitbuf = 0;		// битовый буфер
    uint8_t symbol;		    //текущий проверяемый символ из входа
    uint16_t symb_next; 		// след. байт подгрузки - грузит в инт для сджвигов!
    uint8_t priznak = 0;         // префикс
    uint8_t symblen = 0; 		// текущие из массивов
    uint8_t symbcode = 0;  		//код символа
    uint8_t bits = 16;		// длина - битовый буфер
  
    // uint8_t[] tab_mask = new uint8_t[]{0x01,0x03,0x07,0xF,0x1F,0x3F,0x7F,0xFF}; //[8]от 0 !! кол бит 1 2 3 4 5 6 7 8
    // для 3 бит нет символа!!  для кодирования tab_mask 9 байт!!!!!!!!!!!!! нет 0х00 в начале
    //     uint16_t[] tab_maskWord = new uint16_t[] {0x8000,0xC000,0xE000,0xF000,0xF800,0xFC00,0xFE00,0xFF00};
    //     uint8_t[] tabl_haf_pr = new uint8_t[]{0x80,0x20,0x30,0x0C,0x08,0x0A,0x00,0x38,0x01,0x40}; // [10]
    // дельта должна быть 01 00      05 00 00 00 00 00 02 00      00 00 00 00 00 00 00 00 00 00 00 00
    // из выхода  01 00    41 7e 1f ff ff
    // 41 7e 1f ff ff   01 00 0001 01 11 111   0 000 1    1 111 1111 1111 1  111 1111
    //                  pr|     05   |5 шт 00| 02     | 00 | 12 шт 00       |
    // 01 00    41 7e 1e 0c 18 30 60 ff

    // сначала 2 байта used без сжатия
    symbol = buff_in[bufpos];      //  2 байта lsb msb used
    bufpos++;
    // запись байта в буфер
    buff_out[bufpos_out] = symbol;
    bufpos_out++;
    symbol = buff_in[bufpos];         //если 2 байта
    bufpos++;
    // запись байта в буфер
    buff_out[bufpos_out] = symbol;
    bufpos_out++;

   //чтение 2 байта из входного буфера в bitbuf msb lsb!!! при коде ТОЖЕ!!
#ifdef DEBUG_PRINT
    std::cerr << "dec_Hafman: msb buff_in[bufpos] = " << (int) buff_in[bufpos] << std::endl;
#endif
    // первая загрузка сразу 2 байта - далее по одному
    bitbuf = (uint16_t)(buff_in[bufpos] * 0x100);
    bufpos++;
#ifdef DEBUG_PRINT
    std::cerr <<"dec_Hafman: lsb buff_in[bufpos+1] = " << (int) buff_in[bufpos] << std::endl;
#endif
    bitbuf |= buff_in[bufpos];      //lsb второй (остальные подргужаются в msb!)
    bufpos++;
 
#ifdef DEBUG_PRINT
    std::cerr << "dec_Hafman: начало bufpos = " << (int) bufpos << " bitbuf = " << (int) bitbuf << std::endl;
#endif

    // bufpos = 4  bitbuf = 41 7e //bufpos - индекс во входном буфере !!
    
     while (bufpos < (lenComp + 2))         // <11  bufpos можен возрасти +2 за цикл!? и не весь разобран! лишняя загр. тк слово!
     {
        // if (fdbg) LOG.log_str(String.Format("dec_Hafman: while bufpos={0:D} bitbuf={1:X4}", bufpos, bitbuf));
        // поиск в табл. символа с мл. бита bitbuf! далее подгрузка в ст.байт! если менее 8
        // 8 bit быть не может тк макс.длина =7 !?
        // декод начинает с верху! полный символ    0      8 bit      7     6       5      4        3       2       1
        uint16_t tabMaskWord[] = { 0x0000, 0xFF00, 0xFE00, 0xFC00, 0xF800, 0xF000, 0xE000, 0xC000, 0x8000 };
        //                                     0    1  2  3     4  5  6  7
        uint8_t baseHaf[] = { 0x3f, 1, 1, 2,    2, 1, 1, 0 }; //[0..7] ?=0x3f для декода
        uint8_t offsHaf[] = { 0x3f, 0, 1, 0x3f, 2, 4, 7, 8 }; //[0..7] для декода
        uint8_t symbHaf[] = { 0x00, 0x08, 0x01, 0xff, 0x02, 0xfd, 0xfe, 0x03, 0x04, 0xfc }; //[0..9]
        // private uint8_t[] symblenHaf = new uint8_t[] { 0x01, 0x02, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x07, 0x07 }; //для код [0..9]
        // private uint8_t[] hafCanon = new uint8_t[]   { 0x01, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0x01, 0x00, 0x01 };  //биты []
        // hafCanon = new uint8_t[] {0b1,0b01,0b0010,0b0011,0b00001,0b00010,0x00011,0b000001,0b0000000,0x0000001};  //биты []
        // замена tabl_haf = hafCanon, tabl_len=symblenHaf,tabl_haf_symbol = symbHaf

        int symblenInd = 1;
        for (j = 8; j > 0; j--) {  // 8 7 6 5  4 3 2 1 поиск символа
            //symblenInd++;  // 1 2 3 4 5 6 7 8
            uint16_t tempSymb = (uint16_t)(bitbuf & tabMaskWord[j]);  //начало с конца таб.!! 0х8000
//if (fdbg) LOG.log_str(String.Format("dec_Hafman: tempSymb={0:X4} tab_maskWord[j]={1:X4}", tempSymb, tabMaskWord[j]));
            symbol = (uint8_t)(tempSymb >> (8 + j - 1));  // сдвиг 15 14 13 12  11 10 9 8   (7 6 5  4 3 2 1 0)
            if (symblenInd >= 8) {  //error!! слишком длинный!
//if (fdbg) LOG.log_str("dec_Hafman: symblenInd >= 8 после поиска по табл.");
                return 0;    // из while !!! не найден вообще!!
            }
            if (symbol < baseHaf[symblenInd]) { //   0 01 00000101 1 1 1 ... < 1
                symblenInd++;  //symblen  2 3 4 5 6 7  8 9
                continue;   //еще не найден 
            }
            else
                break; // symblen  1 2 3 4 5 6 7   8  найден в табл.
        } // for j = 8
        // инд  симв
        //  0     0                     [1]=0        1               [1]=1      0+1-1=0
        //  1     8                     [2]=1        01              [2]=1      1+1-1=1

        //  2     1                     [4]=2        0010            [4]=2      2+2-2=2
        //  3     ff                    [4]=2        0011            [4]=2      2+3-2=3

        //  4     2                     [5]=4        00001            [5]=1     4+1-1=4
        //  5     fd                    [5]=4        00010            [5]=1     4+2-1=5
        //  6     fe                    [5]=4        00011            [5]=1     4+3-1=6

        //  7     3                     [6]=7        000001           [6]=1     7+1-1=7

        //  8     4                     [7]=8        0000000          [7]=0     8+0-0=8
        //  9     fc                    [7]=8        0000001          [7]=0     8+1-0=9
        // выделение символа
        symbcode = symbHaf[offsHaf[symblenInd] + symbol - baseHaf[symblenInd]]; // [2 + 0010 - 2] symb[2]
        // symblen = symblenHaf[symblenInd]; // из таблицы!
        symblen = (uint8_t)symblenInd;

        // if (fdbg) LOG.log_str(String.Format("dec_Hafman: symbol={0:X2} symblen={1:X2} symbcode={2:X2}", symbol, symblen, symbcode));
        if(symbcode == 0x08)
            priznak = 1;       // префикс - читать след. байт полностью!!

        bitbuf = (uint16_t)(bitbuf << symblen);      // сдвинуть битбуф.
        bits = (uint8_t)(bits - symblen);
        // на проверку длины и подгрузку и признак
        // if (fdbg) LOG.log_str(String.Format("dec_Hafman: bitbuf={0:X4} symbcode={1:X2} bits={2:D}", bitbuf, symbcode, bits));
        // если был признак - может быть уже сдвинуто - сохранять сам признак не надо!!
        if (bits < 8) {  //заранее подгрузить и выровнять байт!!
        // if (fdbg) LOG.log_str("dec_Hafman: нет priz bits < 8");
            symb_next = (uint16_t)(buff_in[bufpos]);    //
            bufpos++;                       //сдесь увелич. bufpos для while!
            // if (fdbg) LOG.log_str(String.Format("dec_Hafman: нет priz bufpos={0:D}", bufpos));
            symb_next = (uint16_t)(symb_next << (8 - bits));   //
            bitbuf |= (uint16_t)symb_next;
            bits = (uint8_t)(bits + 8);
            // if (fdbg) LOG.log_str(String.Format("dec_Hafman: symb_next={0:X4} bits={1:D}", symb_next, bits));
        }
        //
        if(priznak == 1) {     // префикс длина 2 и биты 10 -взять след.8 бит на выход
            // if (fdbg) LOG.log_str("dec_Hafman: priznak == 1");
            priznak = 0;
            symbcode = (uint8_t)((bitbuf & 0xFF00) >> 8);  //

            bits = (uint8_t)(bits - 8);
            bitbuf = (uint16_t)(bitbuf << 8);
//if (fdbg) LOG.log_str(String.Format("dec_Hafman: priz=1 bitbuf={0:X4}", bitbuf));

            buff_out[bufpos_out] = symbcode;  // запись байта в буфер
            // if (fdbg) LOG.log_str(String.Format("dec_Hafman: priz=1 bufpos_out={0:D} symbcode={1:X2}", bufpos_out, symbcode));
            //  сохранить буфер 1 памяти
            bufpos_out++;
            if (bits < 8)   // подгрузить и выровнять байт!! в lsb !!! bits 16-2-8=6
            {
                // if (fdbg) LOG.log_str("dec_Hafman: priz=1 bits < 8");
                symb_next = (uint16_t)(buff_in[bufpos]);    //
                // if (fdbg) LOG.log_str(String.Format("dec_Hafman: priz=1 symb_next={0:X4} bits={1:D}", symb_next, bits));
                bufpos++;  //сдесь увелич. bufpos для while! может второй раз за цикл!!!!! 
                // if (fdbg) LOG.log_str(String.Format("dec_Hafman: priz=1 bufpos={0:D}", bufpos));
                symb_next = (uint16_t)(symb_next << (8 - bits));   //
                bitbuf |= (uint16_t)symb_next;     //
                bits = (uint8_t)(bits + 8);         //
                // if (fdbg) LOG.log_str(String.Format("dec_Hafman: priz=1 bitbuf={0:X4}", bitbuf));
                // if (fdbg) LOG.log_str(String.Format("dec_Hafman: priz=1 symb_next={0:X4} bits={1:D}", symb_next, bits));
            }
        } else {      // нет признака - сам символ сохранить
            // if (fdbg) LOG.log_str(String.Format("dec_Hafman: no priz bits={0:D}", bits));
            // bitbuf сдвинут bits уменьшен байт подгружен
            // запись байта в буфер
            buff_out[bufpos_out] = symbcode;
            // if (fdbg) LOG.log_str(String.Format("dec_Hafman: no priz bufpos_out={0:D} symbcode={1:X2}", bufpos_out, symbcode));
            bufpos_out++;
            // if (fdbg) LOG.log_str(String.Format("dec_Hafman: no priz bitbuf={0:X4}", bitbuf));
        }
     }  //while
     // буфер входной пуст
     // сохранить буфер в памяти
#ifdef DEBUG_PRINT
    std::cerr << "dec_Hafman: выход" << std::endl;
#endif
    return bufpos_out;      //  bufpos_out+1 кол. зап.символов (включая used)
}

std::string compressLoggerString(const std::string &value)
{
    std::string retval;
    retval.reserve(value.size() * 2);
    uint16_t sz = compressLogger(value.c_str(), (char *) retval.c_str(), value.size());
    retval.resize(sz);
    return retval;
}

std::string decompressLoggerString(const std::string &value)
{
    std::string retval;
    retval.reserve(value.size() * 2);
    uint16_t sz = decompressLogger(value.c_str(), (char *) retval.c_str(), value.size());
    retval.resize(sz);
    return retval;
}
