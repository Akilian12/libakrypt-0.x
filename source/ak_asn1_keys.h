/* ----------------------------------------------------------------------------------------------- */
/*  Copyright (c) 2020 by Axel Kenzo, axelkenzo@mail.ru                                            */
/*                                                                                                 */
/*  Файл ak_asn1_keys.h                                                                            */
/*  - содержит описания функций, предназначенных для экспорта/импорта ключевой информации          */
/* ----------------------------------------------------------------------------------------------- */
#ifndef __AK_ASN1_KEYS_H__
#define __AK_ASN1_KEYS_H__

/* ----------------------------------------------------------------------------------------------- */
 #include <ak_asn1.h>
 #include <ak_sign.h>

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Добавление временного интервала, в ходе которого действительна ключевая информация. */
 int ak_asn1_context_add_time_validity( ak_asn1 , time_t, time_t );
/*! \brief Добавление метаданных секретного ключа. */
 int ak_asn1_context_add_skey_metadata( ak_asn1 , ak_skey );
/*! \brief Создание ASN.1 дерева из заданного файла. */
 int ak_asn1_context_create_from_derfile( ak_asn1 , const char * );

/* ----------------------------------------------------------------------------------------------- */
         /* Функции экспорта/импорта ключевой информации с использованием формата ASN.1 */
/* ----------------------------------------------------------------------------------------------- */
/*! \brief Экспорт секретного ключа в формате ASN.1 дерева. */
 int ak_skey_context_export_to_asn1_with_password( ak_skey , ak_asn1 ,
                                                                     const char * , const size_t );
/*! \brief Экспорт секретного ключа в файл. */
 int ak_skey_context_export_to_derfile_with_password( ak_skey, char * , const size_t ,
                                                                     const char * , const size_t );
/*! \brief Экспорт секретного ключа алгоритма блочного шифрования в файл */
 int ak_bckey_context_export_to_derfile_with_password( ak_bckey , char * , const size_t ,
                                                                     const char * , const size_t );
/*! \brief Экспорт секретного ключа электронной подписи в файл */
 int ak_signkey_context_export_to_derfile_with_password( ak_signkey , char * , const size_t ,
                                                                     const char * , const size_t );
/*! \brief Функция проверяет, что данное ASN.1 дерево является ключевым контейнером. */
 bool_t ak_asn1_context_check_key_container( ak_asn1 , size_t * );
/*! \brief Функция получает ASN.1 дерево, содержащее ключ с заданным порядковым номером. */
 int ak_asn1_context_get_asn1_key_from_container( ak_asn1 , ak_asn1 *, size_t , ak_oid * );
/*! \brief Функция получает метаданные секретного ключа. */
 int ak_asn1_context_get_skey_metadata( ak_asn1 , ak_skey );
/*! \brief Функция создает ключ блочного алгоритма шифрования из заданного ASN.1 дерева. */
 int ak_bckey_context_create_asn1( ak_bckey , ak_asn1 , ak_oid );

#endif
/* ----------------------------------------------------------------------------------------------- */
/*                                                                                 ak_asn1_keys.h  */
/* ----------------------------------------------------------------------------------------------- */