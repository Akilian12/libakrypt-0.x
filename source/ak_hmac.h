/* ----------------------------------------------------------------------------------------------- */
/*  Copyright (c) 2014 - 2018 by Axel Kenzo, axelkenzo@mail.ru                                     */
/*                                                                                                 */
/*  Файл ak_hmac.h                                                                                 */
/*  - содержит описания функций, реализующих семейство ключевых алгоритмов хеширования HMAC.       */
/* ----------------------------------------------------------------------------------------------- */
#ifndef __AK_HMAC_H__
#define __AK_HMAC_H__

/* ----------------------------------------------------------------------------------------------- */
 #include <ak_skey.h>

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Секретный ключ алгоритма выработки имитовставки HMAC. */
/*!  Алгоритм выработки имитовставки HMAC основан на двукратном применении бесключевой функции
     хеширования. Алгоритм описывается рекомендациями IETF RFC 2104 (см. также RFC 7836) и
     стандартизован отечественными рекомендациями по стандартизации Р 50.1.113-2016.
     Алгоритм предназначен, в основном, для выработки имитовставки и преобразования ключевой
     информации.

     В нашей реализации алгоритм может быть использован совместно с любой функцией хеширования,
     реализованной в библиотеке. Отметим, что согласно Р 50.1.113-2016 алгоритм рекомендуется
     использовать только совместно с функцией хеширования Стрибог
     (с длиной хеш кода как 256 бит, так и 512 бит).

     \b Внимание! Использование ключей, чья длина превышает размер блока бесключевой функции
     хеширования, реализовано в соответствии с RFC 2104.                                           */
/* ----------------------------------------------------------------------------------------------- */
 typedef struct hmac {
 /*! \brief контекст секретного ключа */
  struct skey key;
 /*! \brief контекст функции хеширования */
  struct hash ctx;
} *ak_hmac;

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Создание контекста ключевой функции хеширования HMAC на основе функции Стреебог256. */
 int ak_hmac_context_create_streebog256( ak_hmac );
/*! \brief Создание контекста ключевой функции хеширования HMAC на основе функции Стреебог512. */
 int ak_hmac_context_create_streebog512( ak_hmac );
/*! \brief Создание контекста ключевой функции хеширования HMAC на основе функции ГОСТ Р 34.11-94
    с таблицами замен из RFC 4357. */
 int ak_hmac_context_create_gosthash94( ak_hmac );
/*! \brief Создание контекста ключевой функции хеширования HMAC c помощью заданного oid. */
 int ak_hmac_context_create_oid( ak_hmac , ak_oid );
/*! \brief Уничтожение контекста функции хеширования. */
 int ak_hmac_context_destroy( ak_hmac );
/*! \brief Освобождение памяти из под контекста функции хеширования. */
 ak_pointer ak_hmac_context_delete( ak_pointer );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Присвоение секретному ключу константного значения. */
 int ak_hmac_context_set_key( ak_hmac , const ak_pointer , const size_t , const ak_bool );
/*! \brief Присвоение секретному ключу случайного значения. */
 int ak_hmac_context_set_key_random( ak_hmac , ak_random );
/*! \brief Присвоение секретному ключу значения, выработанного из пароля */
 int ak_hmac_context_set_key_from_password( ak_hmac , const ak_pointer , const size_t ,
                                                                 const ak_pointer , const size_t );
/*! \brief Очистка контекста секретного ключа алгоритма выработки имитовставки HMAC, а также
    проверка ресурса ключа. */
 int ak_hmac_context_clean( ak_pointer );
/*! \brief Обновление текущее состояние контекста алгоритма выработки имитовставки HMAC. */
 int ak_hmac_context_update( ak_pointer ctx, const ak_pointer , const size_t );
/*! \brief Завершение алгоритма выработки имитовставки HMAC. */
 ak_buffer ak_hmac_context_finalize( ak_pointer , const ak_pointer , const size_t , ak_pointer );
/*! \brief Вычисление имитовставки для заданной области памяти. */
 ak_buffer ak_hmac_context_ptr( ak_hmac , const ak_pointer , const size_t , ak_pointer );
/*! \brief Вычисление имитовставки для заданного файла. */
 ak_buffer ak_hmac_context_file( ak_hmac , const char*, ak_pointer );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Развертка ключевого вектора из пароля (согласно Р 50.1.111-2016, раздел 4) */
 int ak_hmac_context_pbkdf2_streebog512( const ak_pointer , const size_t ,
                   const ak_pointer , const size_t, const size_t , const size_t , ak_pointer );
/*! \brief Тестирование алгоритмов выработки имитовставки HMAC с отечественными
    функциями хеширования. */
 ak_bool ak_hmac_test_streebog( void );
/*! \brief Тестирование алгоритма PBKDF2, регламентируемого Р 50.1.113-2016. */
 ak_bool ak_hmac_test_pbkdf2( void );

#endif
/* ----------------------------------------------------------------------------------------------- */
/*                                                                                      ak_hmac.h  */
/* ----------------------------------------------------------------------------------------------- */
