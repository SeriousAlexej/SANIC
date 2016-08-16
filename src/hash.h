#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

constexpr unsigned HASH(const char *str) {
  unsigned b    = 378551;
  unsigned a    = 63689;
  unsigned hash = 0;

  for (unsigned i = 0; str[i]; i++) {
      hash = hash * a + str[i];
      a    = a * b;
  }

  return (hash & 0x7FFFFFFF);
}


#endif // HASH_H_INCLUDED
