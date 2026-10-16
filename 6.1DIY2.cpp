
#include <iostream>
#include <numeric>
using namespace std;

class Fraction {
    int numerator, denominator;

public:
    Fraction(int n = 0, int d = 1) {
        if (d == 0) {
            numerator = 0;
            denominator = 1;
            return;
        }

        if (d < 0) {
            n = -n;
            d = -d;
        }

        int g = gcd(n < 0 ? -n : n, d);

        numerator = n / g;
        denominator = d / g;
    }

    friend istream& operator>>(istream& in,
                               Fraction& f) {
        in >> f.numerator >> f.denominator;

        if (f.denominator != 0) {
            f = Fraction(f.numerator, f.denominator);
        }

        return in;
    }

    friend ostream& operator<<(ostream& out,
                               const Fraction& f) {
        return out << f.numerator << "/"
                   << f.denominator;
    }
};

int main() {
    Fraction f;

    cout << "Enter numerator and denominator: ";
    cin >> f;

    cout << "Simplified fraction: " << f << endl;

    return 0;
}
