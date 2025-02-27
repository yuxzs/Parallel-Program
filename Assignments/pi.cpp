#include <iostream>
#include <random>

using namespace std;
long long number_in_circle = 0;
long long number_of_tosses = 100000000;
long long toss = 0;
int main(){

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dist(0.0, 2.0);
    // cout << number_of_tosses << endl;
    double x, y, distance_squared;
    // cout << rand() << endl;
    // cout << rand();
    for (toss = 0; toss < number_of_tosses; toss++){
        // x = (1 - (-1)) * rand() / (RAND_MAX + 1.0) + (-1);
        // y = (1 - (-1)) * rand() / (RAND_MAX + 1.0) + (-1);
        x = dist(gen) - 1.0;
        y = dist(gen) - 1.0;
        distance_squared = x*x + y*y;
        // cout << distance_squared << ' ';
        if ( distance_squared <= 1)
            number_in_circle++;
        // cout << number_in_circle << endl;
    }

    double pi_estimate = 4 * number_in_circle/ ((double) number_of_tosses);
    cout << pi_estimate << endl;
    return 0;
}