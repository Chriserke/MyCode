#include <stdio.h>

double circularBuffer[5];
int circularBufferSize = 5; // Or use: sizeof(circularBuffer) / sizeof(circularBuffer[0]);
int circularIndex = 0;

void addToBuffer(double value);
double getAvgFromBuffer();

int main() {
    // Example usage:
    addToBuffer(1.9); // Because we add 6 numbers to an array of 5 items, this number will be overwritten.
    addToBuffer(2.5);
    addToBuffer(3.25);
    addToBuffer(4);
    addToBuffer(5.6);
    addToBuffer(6.2);

    double average = getAvgFromBuffer(); // 2.5 + 3.25 + 4 + 5.6 + 6.2 = 4.31
    printf("The average is: %f", average);

    return 0;
}

void addToBuffer(double value) {
    // Set the new value in the next spot.
    circularBuffer[circularIndex] = value;

    // Add one to the index so it contains the position of the next spot.
    circularIndex++;
    // If the next spot is outside the boundaries of the array, reset it to the beginning.
    if (circularIndex > circularBufferSize - 1)
        circularIndex = 0;
}

double getAvgFromBuffer() {
    // Loop through the buffer and get a sum of all the values.
    double total = 0.0;
    for (int i = 0; i < circularBufferSize; i++) {
        total += circularBuffer[i];
    }

    return total / circularBufferSize;
}
