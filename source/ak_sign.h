/* ----------------------------------------------------------------------------------------------- */
/*  Copyright (c) 2014 - 2019 by Axel Kenzo, axelkenzo@mail.ru                                     */
/*                                                                                                 */
/*  Файл ak_sign.h                                                                                 */
/*  - содержит описание функций для работы с электронной подписью.                                 */
/* ----------------------------------------------------------------------------------------------- */
#ifndef    __AK_SIGN_H__
#define    __AK_SIGN_H__

/* ----------------------------------------------------------------------------------------------- */
 #include <ak_hmac.h>
 #include <ak_curves.h>

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Секретный ключ алгоритма выработки электронной подписи ГОСТ Р 34.10-2012.

   Ключ может рассматриваться в качестве секретного ключа как для действующего стандарта
   ГОСТ Р 34.10-2012, так и для предыдущей редакции 2001 года. Кроме того, данный контекст
   секретного ключа может быть применим для любого асимметричного криптографического механизма,
   использующего вычисления с эллиптическими кривыми в короткой форме Вейерштрасса, либо
   в искривленной форме Эдвардса. Кривые указанных форм поддерживаются отечественными
   рекомендациями Р 1323565.024-2019.                                                              */
/* ----------------------------------------------------------------------------------------------- */
 typedef struct signkey {
 /*! \brief контекст секретного ключа */
  struct skey key;
 /*! \brief контекст функции хеширования */
  struct hash ctx;
} *ak_signkey;

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция создания контекста секретного ключа электронной подписи. */
 typedef int ( ak_function_create_signkey ) ( ak_signkey , const ak_wcurve );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Инициализация контекста секретного ключа алгоритма ГОСТ Р 34.10-2012. */
 int ak_signkey_context_create_streebog256( ak_signkey , const ak_wcurve );
/*! \brief Инициализация контекста секретного ключа алгоритма ГОСТ Р 34.10-2012. */
 int ak_signkey_context_create_streebog512( ak_signkey , const ak_wcurve );
/*! \brief Инициализация контекста секретного ключа алгоритма выработки электронной подписи
    по заданным идентификаторам алгоритма и эллиптической кривой. */
 int ak_signkey_context_create_oid( ak_signkey , ak_oid , ak_oid );
/*! \brief Уничтожение контекста секретного ключа. */
 int ak_signkey_context_destroy( ak_signkey );
/*! \brief Освобождение памяти из под контекста секретного ключа. */
 ak_pointer ak_signkey_context_delete( ak_pointer );
/*! \brief Размер области памяти, которую занимает электронная подпись. */
 size_t ak_signkey_context_get_tag_size( ak_signkey );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Присвоение секретному ключу электронной подписи константного значения. */
 int ak_signkey_context_set_key( ak_signkey , const ak_pointer , const size_t , const bool_t );
/*! \brief Присвоение секретному ключу электронной подписи случайного значения. */
 int ak_signkey_context_set_key_random( ak_signkey , ak_random );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Выработка электронной подписи для фиксированного значения случайного числа и вычисленного
    заранее значения хеш-функции. */
 void ak_signkey_context_sign_const_values( ak_signkey , ak_uint64 * , ak_uint64 * , ak_pointer );
/*! \brief Выработка электронной подписи для вычисленного заранее значения хеш-функции. */
 int ak_signkey_context_sign_hash( ak_signkey , ak_pointer , size_t , ak_pointer , size_t );
/*! \brief Выработка электронной подписи для заданной области памяти. */
 int ak_signkey_context_sign_ptr( ak_signkey , const ak_pointer , const size_t , ak_pointer , size_t );
/*! \brief Выработка электронной подписи для заданного файла. */
 int ak_signkey_context_sign_file( ak_signkey , const char * , ak_pointer , size_t );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Открытый ключ алгоритма проверки электронной подписи ГОСТ Р 34.10-2012.

   Ключ может рассматриваться в качестве открытого ключа как для действующего стандарта
   ГОСТ Р 34.10-2012, так и для предыдущей редакции 2001 года. Кроме того, данный контекст
   открытого ключа может быть применим для любого асимметричного криптографического механизма,
   использующего вычисления с эллиптическими кривыми в короткой форме Вейерштрасса, либо
   в искривленной форме Эдвардса. Кривые указанных форм поддерживаются отечественными
   рекомендациями Р 1323565.024-2019.                                                              */
/* ----------------------------------------------------------------------------------------------- */
 typedef struct verifykey {
 /*! \brief контекст функции хеширования */
  struct hash ctx;
 /*! \brief контекст эллиптической кривой */
  ak_wcurve wc;
 /*! \brief OID алгоритма проверки */
  ak_oid oid;
 /*! \brief точка кривой, являющаяся открытым ключом электронной подписи */
  struct wpoint qpoint;
} *ak_verifykey;

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Инициализация контекста открытого ключа алгоритма ГОСТ Р 34.10-2012. */
 int ak_verifykey_context_create_from_signkey( ak_verifykey , ak_signkey );
/*! \brief Инициализация контекста открытого ключа алгоритма ГОСТ Р 34.10-2012. */
 int ak_verifykey_context_create_from_ptr( ak_verifykey , ak_pointer , size_t , const ak_wcurve );
/*! \brief Экспорт ключа в виде последовательности байт (формат raw key). */
 int ak_verify_context_export_ptr( ak_verifykey, ak_pointer , size_t );
/*! \brief Уничтожение контекста открытого ключа. */
 int ak_verifykey_context_destroy( ak_verifykey );
/*! \brief Освобождение памяти из под контекста открытого ключа. */
 ak_pointer ak_verifykey_context_delete( ak_pointer );
/*! \brief Проверка электронной подписи для вычисленного заранее значения хеш-функции. */
 bool_t ak_verifykey_context_verify_hash( ak_verifykey , const ak_pointer ,
                                                                       const size_t , ak_pointer );
/*! \brief Проверка электронной подписи для заданной области памяти. */
 bool_t ak_verifykey_context_verify_ptr( ak_verifykey , const ak_pointer ,
                                                                       const size_t , ak_pointer );
/*! \brief Проверка электронной подписи для заданного файла. */
 bool_t ak_verifykey_context_verify_file( ak_verifykey , const char * , ak_pointer );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Выполнение тестовых примеров для алгоритмов выработки и проверки электронной подписи */
 bool_t ak_signkey_test( void );

#endif
/* ----------------------------------------------------------------------------------------------- */
/*                                                                                      ak_sign.h  */
/* ----------------------------------------------------------------------------------------------- */