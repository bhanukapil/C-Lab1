
#include <iostream>
using namespace std;

class Money {
    int rupees, paise;

public:
    Money(int r = 0, int p = 0) {
       rupees = r + p / 100;
        paise = p % 100;
    }

    Money operator+(Money m) {
    return Money(rupees + m.rupees,
     paise + m.paise);
    }

    Money operator-(Money m) {
        int total1 = rupees * 100 + paise;
        int total2 = m.rupees * 100 + m.paise;

        int diff = total1 - total2;
        return Money(diff / 100, diff % 100);
    }

    void display() {
        cout << rupees << " rupees "
             << paise << " paise" << endl;
    }
};

int main() {
    Money m1(10, 80), m2(5, 50);

    Money sum = m1 + m2;
    Money diff = m1 - m2;

    cout << "Addition: ";
    sum.display();

    cout << "Subtraction: ";
    diff.display();

    return 0;
}
