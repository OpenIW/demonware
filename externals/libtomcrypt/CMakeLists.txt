cmake_minimum_required(VERSION 3.22)

add_library(libtomcrypt
  src/ciphers/des.c
  src/ciphers/aes/aes.c
  src/hashes/md5.c
  src/hashes/sha1.c
  src/hashes/tiger.c
  src/hashes/helper/hash_memory.c
  src/hashes/sha2/sha256.c
  src/mac/hmac/hmac_done.c
  src/mac/hmac/hmac_init.c
  src/mac/hmac/hmac_process.c
  src/math/ltm_desc.c
  src/math/multi.c
  src/math/rand_prime.c
  src/misc/error_to_string.c
  src/misc/zeromem.c
  src/misc/crypt/crypt_argchk.c
  src/misc/crypt/crypt_cipher_descriptor.c
  src/misc/crypt/crypt_cipher_is_valid.c
  src/misc/crypt/crypt_find_cipher.c
  src/misc/crypt/crypt_find_cipher_id.c
  src/misc/crypt/crypt_find_hash.c
  src/misc/crypt/crypt_find_prng.c
  src/misc/crypt/crypt_hash_descriptor.c
  src/misc/crypt/crypt_hash_is_valid.c
  src/misc/crypt/crypt_ltc_mp_descriptor.c
  src/misc/crypt/crypt_prng_descriptor.c
  src/misc/crypt/crypt_prng_is_valid.c
  src/misc/crypt/crypt_register_cipher.c
  src/misc/crypt/crypt_register_hash.c
  src/misc/crypt/crypt_register_prng.c
  src/misc/crypt/crypt_unregister_cipher.c
  src/modes/cbc/cbc_decrypt.c
  src/modes/cbc/cbc_encrypt.c
  src/modes/cbc/cbc_setiv.c
  src/modes/cbc/cbc_start.c
  src/modes/ctr/ctr_decrypt.c
  src/modes/ctr/ctr_done.c
  src/modes/ctr/ctr_encrypt.c
  src/modes/ctr/ctr_start.c
  src/pk/asn1/der/bit/der_decode_bit_string.c
  src/pk/asn1/der/bit/der_encode_bit_string.c
  src/pk/asn1/der/bit/der_length_bit_string.c
  src/pk/asn1/der/boolean/der_decode_boolean.c
  src/pk/asn1/der/boolean/der_encode_boolean.c
  src/pk/asn1/der/boolean/der_length_boolean.c
  src/pk/asn1/der/choice/der_decode_choice.c
  src/pk/asn1/der/ia5/der_decode_ia5_string.c
  src/pk/asn1/der/ia5/der_encode_ia5_string.c
  src/pk/asn1/der/ia5/der_length_ia5_string.c
  src/pk/asn1/der/integer/der_decode_integer.c
  src/pk/asn1/der/integer/der_encode_integer.c
  src/pk/asn1/der/integer/der_length_integer.c
  src/pk/asn1/der/object_identifier/der_decode_object_identifier.c
  src/pk/asn1/der/object_identifier/der_encode_object_identifier.c
  src/pk/asn1/der/object_identifier/der_length_object_identifier.c
  src/pk/asn1/der/octet/der_decode_octet_string.c
  src/pk/asn1/der/octet/der_encode_octet_string.c
  src/pk/asn1/der/octet/der_length_octet_string.c
  src/pk/asn1/der/printable_string/der_decode_printable_string.c
  src/pk/asn1/der/printable_string/der_encode_printable_string.c
  src/pk/asn1/der/printable_string/der_length_printable_string.c
  src/pk/asn1/der/sequence/der_decode_sequence_ex.c
  src/pk/asn1/der/sequence/der_decode_sequence_multi.c
  src/pk/asn1/der/sequence/der_encode_sequence_ex.c
  src/pk/asn1/der/sequence/der_encode_sequence_multi.c
  src/pk/asn1/der/sequence/der_length_sequence.c
  src/pk/asn1/der/set/der_encode_set.c
  src/pk/asn1/der/set/der_encode_setof.c
  src/pk/asn1/der/short_integer/der_decode_short_integer.c
  src/pk/asn1/der/short_integer/der_encode_short_integer.c
  src/pk/asn1/der/short_integer/der_length_short_integer.c
  src/pk/asn1/der/utctime/der_decode_utctime.c
  src/pk/asn1/der/utctime/der_encode_utctime.c
  src/pk/asn1/der/utctime/der_length_utctime.c
  src/pk/asn1/der/utf8/der_decode_utf8_string.c
  src/pk/asn1/der/utf8/der_encode_utf8_string.c
  src/pk/asn1/der/utf8/der_length_utf8_string.c
  src/pk/ecc/ecc.c
  src/pk/ecc/ecc_export.c
  src/pk/ecc/ecc_free.c
  src/pk/ecc/ecc_import.c
  src/pk/ecc/ecc_make_key.c
  src/pk/ecc/ecc_shared_secret.c
  src/pk/ecc/ltc_ecc_is_valid_idx.c
  src/pk/ecc/ltc_ecc_map.c
  src/pk/ecc/ltc_ecc_mulmod.c
  src/pk/ecc/ltc_ecc_mul2add.c
  src/pk/ecc/ltc_ecc_points.c
  src/pk/ecc/ltc_ecc_projective_add_point.c
  src/pk/ecc/ltc_ecc_projective_dbl_point.c
  src/pk/pkcs1/pkcs_1_mgf1.c
  src/pk/pkcs1/pkcs_1_oaep_decode.c
  src/pk/pkcs1/pkcs_1_oaep_encode.c
  src/pk/pkcs1/pkcs_1_pss_decode.c
  src/pk/pkcs1/pkcs_1_v1_5_decode.c
  src/pk/pkcs1/pkcs_1_v1_5_encode.c
  src/pk/rsa/rsa_decrypt_key.c
  src/pk/rsa/rsa_encrypt_key.c
  src/pk/rsa/rsa_export.c
  src/pk/rsa/rsa_exptmod.c
  src/pk/rsa/rsa_free.c
  src/pk/rsa/rsa_import.c
  src/pk/rsa/rsa_make_key.c
  src/pk/rsa/rsa_verify_hash.c
  src/prngs/yarrow.c
)

target_include_directories(libtomcrypt
PUBLIC
  src/headers
)

target_compile_definitions(libtomcrypt
PRIVATE
  "_CRT_SECURE_NO_WARNINGS"
)

target_link_libraries(libtomcrypt
PUBLIC
  libtommath
)

# add_compile_definitions(LTM_DESC LTC_SOURCE USE_LTM)



# target_include_directories(${PROJECT_NAME} PUBLIC src/headers)

# add_virtual_path(libtomcrypt)

# target_compile_definitions(libtomcrypt PUBLIC
#     "_DEBUG"
#     "_WINDOWS;"
#     "_CRT_SECURE_NO_WARNINGS;")
