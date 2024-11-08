#include <iostream>
#include <emmintrin.h> // Для работы с __m128 и SSE

int main() {
  // Инициализация зашифрованных данных
  __m128i v31 = _mm_set_epi64x(0xDADA11977ADB0098, 0x9BE82BF6DEDD8D27);
  __m128i v32_encrypted = _mm_set_epi64x(0xDADA11977ADB0098, 0xFE8442B0BABCE875);
  __m128i v33_encrypted = _mm_set_epi64x(0xDADA119716B764B6, 0xA9DB4793B0AFE84C);

  // Расшифровка
  __m128i v32_decrypted = _mm_xor_si128(v31, v32_encrypted);
  __m128i v33_decrypted = _mm_xor_si128(v31, v33_encrypted);

  // Вывод результата
  char v32_result[16];
  char v33_result[16];
  _mm_storeu_si128((__m128i*)v32_result, v32_decrypted);
  _mm_storeu_si128((__m128i*)v33_result, v33_decrypted);

  std::cout << "Расшифрованное имя функции: " << std::string(v32_result, 16) << std::endl;
  std::cout << "Расшифрованное имя DLL: " << std::string(v33_result, 16) << std::endl;

  return 0;
}
