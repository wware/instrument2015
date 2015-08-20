#ifndef PROFILE_H_INCLUDED
#define PROFILE_H_INCLUDED

#include <sstream>
#include <string>
#include <iostream>
using namespace std;

#define NUM_BUCKETS 6

extern uint32_t micros(void);

class Profile
{
    uint32_t buckets[NUM_BUCKETS];
    uint32_t starts[NUM_BUCKETS];
    Profile() {
        int i;
        for (i = 0; i < NUM_BUCKETS; i++)
            buckets[i] = starts[i] = 0;
    }
    void start(uint8_t bucket) {
        starts[bucket] = micros();
    }
    void finish(uint8_t bucket) {
        buckets[bucket] = micros() - starts[bucket];
    }
    std::string results(uint8_t outer, uint8_t inner) {
        // http://stackoverflow.com/questions/2288970
        ostringstream os;
        os << "Outer: " << buckets[outer] << endl;
        os << "Inner: " << buckets[inner] << endl;
        os << "Fraction: " << (1.0 * buckets[inner] / buckets[outer]) << endl;
        return os.str();
    }
};

#endif     // PROFILE_H_INCLUDED
