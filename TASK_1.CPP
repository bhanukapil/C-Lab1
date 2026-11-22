#include <iostream>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <cstddef>
using namespace std;
 

inline double toMilliVolts(double volts) {
    return volts * 1000.0;
}
 
double calculateAverage(const double values[], int size) {
    if (size <= 0) return 0.0;
    double total = 0.0;
    for (int i = 0; i < size; ++i) total += values[i];
    return total / size;
}
 

void swapByReference(double &a, double &b) {
    double temp = a;
    a = b;
    b = temp;
}
 
void swapByPointer(double *a, double *b) {
    if (a == nullptr || b == nullptr) return;
    double temp = *a;
    *a = *b;
    *b = temp;
}
 
class SignalSeries {
private:
    string name;
    double *samples;
    size_t count;
    static int liveObjects; 
 
    void copySamples(const double *source, size_t n) {
        samples = (n > 0) ? new double[n] : nullptr;
        count = n;
        for (size_t i = 0; i < count; ++i) samples[i] = source[i];
    }
 
public:
    
    SignalSeries() : name("Unnamed"), samples(nullptr), count(0) {
        ++liveObjects;
    }
 
    SignalSeries(const string &label, const double values[], size_t n)
        : name(label), samples(nullptr), count(0) {
        copySamples(values, n);
        ++liveObjects;
    }
 
    
    SignalSeries(const SignalSeries &other)
        : name(other.name), samples(nullptr), count(0) {
        copySamples(other.samples, other.count);
        ++liveObjects;
    }
 
    
    SignalSeries &operator=(const SignalSeries &other) {
        if (this != &other) {
            double *newSamples =
                (other.count > 0) ? new double[other.count] : nullptr;
            for (size_t i = 0; i < other.count; ++i)
                newSamples[i] = other.samples[i];
 
            delete[] samples;
            samples = newSamples;
            count = other.count;
            name = other.name;
        }
        return *this;
    }
 
    ~SignalSeries() {
        delete[] samples;
        --liveObjects;
    }
 
    
    void setName(const string &name) {
        this->name = name;
    }
 
    string getName() const {
        return name;
    }
 
    size_t size() const {
        return count;
    }
 
    
    static int getLiveObjects() {
        return liveObjects;
    }
 

    SignalSeries operator+(const SignalSeries &other) const {
        size_t resultSize = (count < other.count) ? count : other.count;
        double *result = (resultSize > 0) ? new double[resultSize] : nullptr;
 
        for (size_t i = 0; i < resultSize; ++i)
            result[i] = samples[i] + other.samples[i];
 
        SignalSeries sum(name + " + " + other.name, result, resultSize);
        delete[] result;
        return sum;
    }
 
    
    bool operator==(const SignalSeries &other) const {
        if (count != other.count) return false;
        for (size_t i = 0; i < count; ++i)
            if (samples[i] != other.samples[i]) return false;
        return true;
    }
 
    
    SignalSeries &operator++() {
        for (size_t i = 0; i < count; ++i) ++samples[i];
        return *this;
    }
 

    SignalSeries operator++(int) {
        SignalSeries old(*this);
        ++(*this);
        return old;
    }
 

    double &operator[](size_t index) {
        if (index >= count) throw out_of_range("Sample index is out of range.");
        return samples[index];
    }
 
    const double &operator[](size_t index) const {
        if (index >= count) throw out_of_range("Sample index is out of range.");
        return samples[index];
    }
 

    friend ostream &operator<<(ostream &out, const SignalSeries &signal);
    friend istream &operator>>(istream &in, SignalSeries &signal);
 

    friend double calculateAverage(const SignalSeries &signal);
};
 

int SignalSeries::liveObjects = 0;
 

ostream &operator<<(ostream &out, const SignalSeries &signal) {
    out << signal.name << " [";
    for (size_t i = 0; i < signal.count; ++i) {
        out << fixed << setprecision(2) << signal.samples[i];
        if (i + 1 < signal.count) out << ", ";
    }
    out << "]";
    return out;
}
 

istream &operator>>(istream &in, SignalSeries &signal) {
    string label;
    int n;
 
    cout << "Enter signal name (one word): ";
    in >> label;
    cout << "Enter number of samples (1-20): ";
    in >> n;
 
    if (!in || n < 1 || n > 20) {
        in.setstate(ios::failbit);
        return in;
    }
 
    double *input = new double[n];
    cout << "Enter " << n << " sample values in mV: ";
    for (int i = 0; i < n; ++i) in >> input[i];
 
    if (in) {
        delete[] signal.samples;
        signal.samples = input;
        signal.count = static_cast<size_t>(n);
        signal.name = label;
    } else {
        delete[] input;
    }
    return in;
}
 

double calculateAverage(const SignalSeries &signal) {
    if (signal.count == 0) return 0.0;
    double total = 0.0;
    for (size_t i = 0; i < signal.count; ++i)
        total += signal.samples[i];
    return total / signal.count;
}
 

void displaySummary(const SignalSeries &signal, const string &unit = "mV") {
    cout << "\nSignal: " << signal.getName() << '\n';
    cout << "Samples: " << signal.size() << '\n';
    cout << "Average: " << calculateAverage(signal) << " " << unit << '\n';
}
 
int main() {
    cout << "========================================\n";
    cout << "       ECE SIGNAL DATA MANAGEMENT\n";
    cout << "========================================\n";
 
    SignalSeries original;
    cout << "\nEnter the signal details:\n";
    if (!(cin >> original)) {
        cout << "Invalid input. Restart and enter 1 to 20 numeric samples.\n";
        return 1;
    }
 
    cout << "\nOriginal data: " << original << '\n';
    displaySummary(original); // uses default argument
 

    SignalSeries copied(original);
    cout << "\nCopied data:   " << copied << '\n';
    cout << "Live SignalSeries objects: "
         << SignalSeries::getLiveObjects() << '\n';
 
    
    SignalSeries combined = original + copied;
    cout << "\nAdded signal:  " << combined << '\n';
    cout << "Original equals copy? "
         << (original == copied ? "Yes" : "No") << '\n';
 
    ++copied;
    cout << "Copy after prefix ++: " << copied << '\n';
    cout << "Original equals updated copy? "
         << (original == copied ? "Yes" : "No") << '\n';
 
    
    if (combined.size() > 0) {
        combined[0] = toMilliVolts(combined[0] / 1000.0);
        cout << "First combined sample using []: "
             << combined[0] << " mV\n";
    }
 
    
    double a = 12.5, b = 7.5;
    swapByReference(a, b);
    cout << "\nAfter reference swap: a=" << a << ", b=" << b << '\n';
    swapByPointer(&a, &b);
    cout << "After pointer swap:   a=" << a << ", b=" << b << '\n';
 
    
    double calibration[] = {10.0, 20.0, 30.0};
    cout << "\nCalibration average: "
         << calculateAverage(calibration, 3) << " mV\n";
 
    cout << "\nObjects currently alive: "
         << SignalSeries::getLiveObjects() << '\n';
    cout << "Program completed successfully.\n";
    return 0;
}
