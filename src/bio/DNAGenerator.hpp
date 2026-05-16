#ifndef DNA_GENERATOR_HPP
#define DNA_GENERATOR_HPP

#include <string>

//deklaracija funkcije koja generira DNA sekvencu zadane duljine koristeci moderni random generator
std::string generateDNA(size_t length);
//deklaracije funkcije koja generira DNA sekvencu koristeci klasicni rand()
std::string generateRandomDNA(int k);

#endif
