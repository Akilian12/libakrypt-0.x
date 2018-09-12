/* ----------------------------------------------------------------------------------------------- */
/*  Copyright (c) 2014 - 2018 by Axel Kenzo, axelkenzo@mail.ru                                     */
/*                                                                                                 */
/*  Файл ak_bckey.h                                                                                */
/*  - содержит реализацию общих функций для алгоритмов блочного шифрования.                        */
/* ----------------------------------------------------------------------------------------------- */
 #include <ak_bckey.h>

/* ----------------------------------------------------------------------------------------------- */
/*! Функция устанавливает параметры алгоритма блочного шифрования, передаваемые в качестве
    аргументов. После инициализации остаются неопределенными следующие поля и методы,
    зависящие от конкретной реализации алгоритма блочного шифрования:

    - bkey.encrypt -- алгоритм зашифрования одного блока
    - bkey.decrypt -- алгоритм расшифрования одного блока
    - bkey.shedule_keys -- алгоритм развертки ключа и генерации раундовых ключей
    - bkey.delete_keys -- функция удаления раундовых ключей

    Следующие поля принимают значения по-умолчанию
    - bkey.key.data -- указатель на служебную область памяти
    - bkey.key.resource.counter -- максимально возможное число обрабатываемых блоков информации
    - bkey.key.oid -- идентификатор алгоритма шифрования
    - bkey.key.set_mask -- функция установки или смены маски ключа
    - bkey.key.unmask -- функция снятия маски с ключа
    - bkey.key.set_icode -- функция вычисления кода целостности
    - bkey.key.check_icode -- функция проверки кода целостности

    Перечисленные методы могут переопределяться в производящих функциях,
    создающих объекты конкретных алгоритмов блочного шифрования.

    @param bkey контекст ключа алгоритма блочного шифрованния
    @param keysize длина ключа в байтах
    @param blocksize длина блока обрабатываемых данных в байтах
    @return В случае успеха функция возвращает \ref ak_error_ok (ноль).
    В противном случае, возвращается код ошибки.                                                   */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_create( ak_bckey bkey, size_t keysize, size_t blocksize )
{
  int error = ak_error_ok;
  if( bkey == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                   "using a null pointer to block cipher context" );
  if( !keysize ) return ak_error_message( ak_error_zero_length, __func__,
                                                        "using block cipher key with zero length" );
  if( !blocksize ) return ak_error_message( ak_error_zero_length, __func__,
                                                            "using cipher with zero block length" );
 /* инициализируем данные,
    для ключей блочного шифрования длина контрольной суммы всегда равна 8 байт */
  if(( error = ak_skey_context_create( &bkey->key, keysize, 8 )) != ak_error_ok )
    return ak_error_message( error, __func__, "wrong creation of secret key" );

 /* структура, хранящая синхропосылку инициализируется нулевым значением */
  if(( error = ak_buffer_create( &bkey->ivector )) != ak_error_ok ) {
    if( ak_skey_context_destroy( &bkey->key ) != ak_error_ok )
      ak_error_message( ak_error_get_value(), __func__, "wrong destroying a secret key" );
    return ak_error_message( error, __func__, "wrong memory allocation for temporary vector");
  }

  bkey->bsize =    blocksize;
  bkey->encrypt =       NULL;
  bkey->decrypt =       NULL;
  bkey->schedule_keys = NULL;
  bkey->delete_keys =   NULL;

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param bkey контекст ключа алгоритма блочного шифрованния
    @return В случае успеха функция возввращает \ref ak_error_ok (ноль).
    В противном случае, возвращается код ошибки.                                                   */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_destroy( ak_bckey bkey )
{
  int error = ak_error_ok;
  if( bkey == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                  "using a null pointer to block cipher context" );
  if( bkey->delete_keys != NULL ) {
    if(( error = bkey->delete_keys( &bkey->key )) != ak_error_ok ) {
      ak_error_message( error, __func__ , "wrong deleting of round keys" );
    }
  }
 /* вектор для хранения синхропосылок может быть не определен */
  if( bkey->ivector.data != NULL ) {
    if(( error = ak_buffer_wipe( &bkey->ivector, &bkey->key.generator )) != ak_error_ok )
      ak_error_message( error, __func__, "wrong wiping a temporary vector");
  }
  if(( error = ak_buffer_destroy( &bkey->ivector )) != ak_error_ok )
    ak_error_message( error, __func__, "wrong destroying a temporary vector" );
  if(( error = ak_skey_context_destroy( &bkey->key )) != ak_error_ok )
    ak_error_message( error, __func__, "wrong destroying a secret key" );

  bkey->bsize =            0;
  bkey->encrypt =       NULL;
  bkey->decrypt =       NULL;
  bkey->schedule_keys = NULL;
  bkey->delete_keys =   NULL;

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param bkey контекст ключа алгоритма блочного шифрованния
    @return Функция всегда возвращает NULL.                                                        */
/* ----------------------------------------------------------------------------------------------- */
 ak_pointer ak_bckey_context_delete( ak_pointer bkey )
{
  if( bkey != NULL ) {
    ak_bckey_context_destroy( bkey );
    free( bkey );
  } else ak_error_message( ak_error_null_pointer, __func__ ,
                                                         "using null pointer to block cipher key" );
 return NULL;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция присваивает контексту ключа алгоритма блочного шифрования заданное значение,
    содержащееся в области памяти, на которую указывает аргумент функции keyptr.
    При инициализации значение ключа \b копируется в контекст ключа, если флаг cflag истиннен.
    Если флаг ложен, то копирования (размножения ключевой информации) не происходит.
    Поведение функции при копировании аналогично поведению функции ak_buffer_set_ptr().

    Перед присвоением ключа контекст должен быть инициализирован.

    После присвоения значения ключа производится его маскирование и выработка контрольной суммы.

    Предпалагается, что основное использование функции ak_bckey_context_set_key()
    заключается в тестировании алгоритма блочного шифрования на заданных (тестовых)
    значениях ключей. Другое использование функции - присвоение значений, выработанных в ходе
    выполнения алгоритмов выработки ключевой информации.

    @param bkey Контекст ключа блочного алгоритма шифрования.
    @param keyptr Указатель на область памяти, содержащую значение ключа.
    @param size Размер области памяти, содержащей значение ключа.
    @param cflag Флаг владения данными. Если он истинен, то данные копируются в область памяти,
    которой владеет ключевой контекст.

    @return Функция возвращает код ошибки. В случае успеха возвращается \ref ak_error_ok.          */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_set_key( ak_bckey bkey,
                                   const ak_pointer keyptr, const size_t size, const ak_bool cflag )
{
  int error = ak_error_ok;

 /* проверяем входные данные */
  if( bkey == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                        "using null pointer to secret key context" );
  if( keyptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                                  "using null pointer to key data" );
  if( size != bkey->key.key.size ) return ak_error_message( ak_error_wrong_length, __func__,
                                         "using a constant value for secret key with wrong length" );
 /* присваиваем ключевой буффер */
  if(( error = ak_skey_context_set_key( &bkey->key, keyptr, size, cflag )) != ak_error_ok )
    return ak_error_message( error, __func__ , "incorrect assigning of key data" );

 /* выполняем развертку раундовых ключей */
  if( bkey->schedule_keys != NULL ) bkey->schedule_keys( &bkey->key );

 return error;
}


/* ----------------------------------------------------------------------------------------------- */
/*                             теперь реализация режимов шифрования                                */
/* ----------------------------------------------------------------------------------------------- */
/*! @param bkey Контекст ключа алгоритма блочного шифрования.
    @param in Указатель на область памяти, где хранятся входные (зашифровываемые) данные
    @param out Указатель на область памяти, куда помещаются зашифрованные данные
    (этот указатель может совпадать с in)
    @param size Размер зашировываемых данных (в байтах). Для режима простой замены
    длина зашифровываемых данных должна быть кратна длине блока.

    @return В случае возникновения ошибки функция возвращает ее код, в противном случае
    возвращается ak_error_ok (ноль)                                                                */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_encrypt_ecb( ak_bckey bkey, ak_pointer in, ak_pointer out, size_t size )
{
  ak_int64 blocks = 0;
  int error = ak_error_ok;
  ak_uint64 *inptr = (ak_uint64 *)in, *outptr = (ak_uint64 *)out;

 /* выполняем проверку размера входных данных */
  if( size%bkey->bsize != 0 )
    return ak_error_message( ak_error_wrong_block_cipher_length,
                            __func__ , "the length of input data is not divided by block length" );

 /* проверяем целостность ключа */
  if( bkey->key.check_icode( &bkey->key ) != ak_true )
    return ak_error_message( ak_error_wrong_key_icode,
                                        __func__, "incorrect integrity code of secret key value" );
 /* уменьшаем значение ресурса ключа */
  blocks = (ak_uint64 ) size/bkey->bsize;
  if( bkey->key.resource.counter < blocks )
    return ak_error_message( ak_error_low_key_resource,
                                                   __func__ , "low resource of block cipher key" );
   else bkey->key.resource.counter -= blocks;

 /* теперь приступаем к зашифрованию данных */
  switch( bkey->bsize ) {
    case  8: /* шифр с длиной блока 64 бита */
      do {
        bkey->encrypt( &bkey->key, inptr++, outptr++ );
      } while( --blocks > 0 );
    break;

    case 16: /* шифр с длиной блока 128 бит */
      do {
        bkey->encrypt( &bkey->key, inptr, outptr );
        inptr+=2; outptr+=2;
      } while( --blocks > 0 );
    break;
    default: return ak_error_message( ak_error_wrong_block_cipher,
                                          __func__ , "incorrect block size of block cipher key" );
  }
 /* перемаскируем ключ */
  if(( error = bkey->key.set_mask( &bkey->key )) != ak_error_ok )
    ak_error_message( error, __func__ , "wrong remasking of secret key" );

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param bkey Контекст ключа алгоритма блочного шифрования.
    @param in Указатель на область памяти, где хранятся входные (расшифровываемые) данные.
    @param out Указатель на область памяти, куда помещаются расшифрованные данные
    (этот указатель может совпадать с in).
    @param size Размер расшировываемых данных (в байтах). Для режима простой замены
    длина расшифровываемых данных должна быть кратна длине блока.

    @return В случае возникновения ошибки функция возвращает ее код, в противном случае
    возвращается ak_error_ok (ноль)                                                                */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_decrypt_ecb( ak_bckey bkey, ak_pointer in, ak_pointer out, size_t size )
{
  ak_int64 blocks = 0;
  int error = ak_error_ok;
  ak_uint64 *inptr = (ak_uint64 *)in, *outptr = (ak_uint64 *)out;

 /* выполняем проверку размера входных данных */
  if( size%bkey->bsize != 0 )
    return ak_error_message( ak_error_wrong_block_cipher_length,
                            __func__ , "the length of input data is not divided by block length" );

 /* проверяем целостность ключа */
  if( bkey->key.check_icode( &bkey->key ) != ak_true )
    return ak_error_message( ak_error_wrong_key_icode,
                                        __func__, "incorrect integrity code of secret key value" );
 /* уменьшаем значение ресурса ключа */
  blocks = (ak_uint64 ) size/bkey->bsize;
  if( bkey->key.resource.counter < blocks )
    return ak_error_message( ak_error_low_key_resource,
                                                   __func__ , "low resource of block cipher key" );
   else bkey->key.resource.counter -= blocks;

 /* теперь приступаем к расшифрованию данных */
  switch( bkey->bsize ) {
    case  8: /* шифр с длиной блока 64 бита */
      do {
        bkey->decrypt( &bkey->key, inptr++, outptr++ );
      } while( --blocks > 0 );
    break;

    case 16: /* шифр с длиной блока 128 бит */
      do {
        bkey->decrypt( &bkey->key, inptr, outptr );
        inptr+=2; outptr+=2;
      } while( --blocks > 0 );
    break;
    default: return ak_error_message( ak_error_wrong_block_cipher,
                                          __func__ , "incorrect block size of block cipher key" );
  }
 /* перемаскируем ключ */
  if(( error = bkey->key.set_mask( &bkey->key )) != ak_error_ok )
    ak_error_message( error, __func__ , "wrong remasking of secret key" );

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param bkey Контекст ключа алгоритма блочного шифрования, на котором происходит
    зашифрование или расшифрование информации.
    @param in Указатель на область памяти, где хранятся входные (открытые) данные.
    @param out Указатель на область памяти, куда помещаются зашифрованные данные
    (этот указатель может совпадать с `in`).
    @param size Размер зашировываемых данных (в байтах).
    @param iv Указатель на произвольную область памяти - синхропосылку. Область памяти, на
    которую указывает `iv` не изменяется.
    @param iv_size Длина синхропосылки в байтах. Согласно  стандарту ГОСТ Р 34.13-2015 длина
    синхропосылки должна быть ровно в два раза меньше, чем длина блока, то есть 4 байта для Магмы
    и 8 байт для Кузнечика. Значение `iv_size`, отличное от указанных, приведет к возникновению
    ошибки.

    @return В случае возникновения ошибки функция возвращает ее код, в противном случае
    возвращается ak_error_ok (ноль)

    Поскольку операцией зашифрования является сложение открытого текста по модулю два
    с последовательностью, вырабатываемой блочным шифром, то операция расшифрования производится
    также наложением гаммы по модулю два. Таким образом, для зашифрования и расшифрования
    информациии используется одна и таже функция.

    Значение синхропосылки копируется (область памяти, на которую указывает iv не изменяется) и,
    в ходе реализации режима гаммирования, преобразуется. Преобразованное значение сохраняется в
    контексте секретного ключа в буффере `skey.ivector`.
    Данное значение может быть использовано при повторном вызове функции ak_bckey_context_xcrypt().
    Следующий пример иллюстрирует использование того факта, что текущее значение синхропосылки
    сохраняется внутри контекста секретного ключа.

\code
 // шифрование буффера с данными одним фрагментом
  ak_bckey_context_xcrypt( key, in, out, size, iv, 4 );

 // тот же результат может быть получен за несколько вызовов
  ak_bckey_context_xcrypt( &key, in, out, 16, iv, 4 );
  ak_bckey_context_xcrypt( &key, in+16, out+16, 16, NULL, 0 );
  ak_bckey_context_xcrypt( &key, in+32, out+32, size-32, NULL, 0 );
        // для того, чтобы использовать внутреннее значение синхропосылки,
        //              мы передаем нулевые значения последних параметров
\endcode

 В данном фрагменте исходный буффер сначала зашифровывается за один вызов функции,
 а потом фрагментами, длина котрых кратна длине блока используемого алгоритма блочного шифрования.
 Результаты зашифрования должны совпадать в обоих случаях. Указанное поведение функции позволяет
 зашифровывать данные в случае, когда они поступаю фрагментами, например из сети, или хранение
 данных полностью в оперативной памяти нецелесообразно (например, шифрование больших файлов).      */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_xcrypt( ak_bckey bkey, ak_pointer in, ak_pointer out, size_t size,
                                                                     ak_pointer iv, size_t iv_size )
{
  int error = ak_error_ok;
  ak_int64 blocks = (ak_int64)size/bkey->bsize,
            tail = (ak_int64)size%bkey->bsize;
  ak_uint64 yaout[2], *inptr = (ak_uint64 *)in, *outptr = (ak_uint64 *)out;

 /* проверяем целостность ключа */
  if( bkey->key.check_icode( &bkey->key ) != ak_true )
    return ak_error_message( ak_error_wrong_key_icode, __func__,
                                                   "incorrect integrity code of secret key value" );
 /* уменьшаем значение ресурса ключа */
  if( bkey->key.resource.counter < ( blocks + ( tail > 0 )))
    return ak_error_message( ak_error_low_key_resource,
                                                    __func__ , "low resource of block cipher key" );
   else bkey->key.resource.counter -= ( blocks + ( tail > 0 ));

 /* выбираем, как вычислять синхропосылку */
  if(( iv == NULL ) || ( iv_size == 0 )) { /* запрос на использование внутреннего значения */
    if( ak_buffer_is_assigned( &bkey->ivector ) != ak_true )
      return ak_error_message( ak_error_wrong_block_cipher_function, __func__ ,
                                  "first calling function with undefined value of initial vector" );
    if( bkey->key.flags&bckey_flag_not_xcrypt )
      return ak_error_message( ak_error_wrong_block_cipher_function, __func__ ,
                              "secondary calling function with undefined value of initial vector" );
  } else {
    /* проверяем длину синхропосылки (если меньше половины блока, то плохо)
        если больше - то лишнее не используется */
     if( iv_size < ( bkey->bsize >> 1 ))
       return ak_error_message( ak_error_wrong_iv_length, __func__,
                                                              "incorrect length of initial value" );
    /* выделяем память под буффер и помещаем в него значение */
     if(( error = ak_buffer_set_size( &bkey->ivector, bkey->bsize )) != ak_error_ok )
       return ak_error_message( error, __func__ , "incorrect momory allocation for internal vector" );

     memset( bkey->ivector.data, 0, bkey->ivector.size );
     memcpy( ((ak_uint8 *)bkey->ivector.data) + (bkey->bsize >> 1), iv, iv_size );
    /* снимаем значение флага */
     if( bkey->key.flags&bckey_flag_not_xcrypt ) bkey->key.flags ^= bckey_flag_not_xcrypt;
    }

 /* обработка основного массива данных (кратного длине блока) */
  switch( bkey->bsize ) {
    case  8: /* шифр с длиной блока 64 бита */
      do {
          bkey->encrypt( &bkey->key, bkey->ivector.data, yaout );
          *outptr = *inptr ^ yaout[0];
          outptr++; inptr++; ((ak_uint64 *)bkey->ivector.data)[0]++;
      } while( --blocks > 0 );
    break;

    case 16: /* шифр с длиной блока 128 бит */
      do {
          bkey->encrypt( &bkey->key, bkey->ivector.data, yaout );
          *outptr = *inptr ^ yaout[0]; outptr++; inptr++;
          *outptr = *inptr ^ yaout[1]; outptr++; inptr++;
          ((ak_uint64 *)bkey->ivector.data)[0]++; // здесь мы не учитываем знак переноса
                                                  // потому что объем данных на одном ключе не должен превышать
                                                  // 2^64 блоков (контролируется через ресурс ключа)
      } while( --blocks > 0 );
    break;

    default: return ak_error_message( ak_error_wrong_block_cipher,
                                          __func__ , "incorrect block size of block cipher key" );
  }

 /* обрабатываем хвост сообщения */
  if( tail ) {
    size_t i;
    bkey->encrypt( &bkey->key, bkey->ivector.data, yaout );
    for( i = 0; i < tail; i++ ) /* теперь мы гаммируем tail байт, используя для этого
                                   старшие байты (most significant bytes) зашифрованного счетчика */
        ( (ak_uint8*)outptr )[i] =
           ( (ak_uint8*)inptr )[i]^( (ak_uint8 *)yaout)[bkey->ivector.size-tail+i];
   /* запрещаем дальнейшее использование xcrypt на данном значении синхропосылки,
                                           поскольку обрабатываемые данные не кратны длине блока. */
    memset( bkey->ivector.data, 0, bkey->ivector.size );
    bkey->key.flags |= bckey_flag_not_xcrypt;
  }

 /* перемаскируем ключ */
  if(( error = bkey->key.set_mask( &bkey->key )) != ak_error_ok )
    ak_error_message( error, __func__ , "wrong remasking of secret key" );

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*!  \example test-internal-bckey01.c                                                              */
/*!  \example test-internal-bckey02.c                                                              */
/*!  \example test-internal-bckey03.c                                                              */
/* ----------------------------------------------------------------------------------------------- */
/*                                                                                     ak_bckey.c  */
/* ----------------------------------------------------------------------------------------------- */
