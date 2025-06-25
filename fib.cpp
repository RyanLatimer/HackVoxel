#include <iostream>

int difference(int n, int m) {
    int diff = m - n;
    if (diff < 0) {
        diff = -diff;
    }
    return diff;
}

int fib(int userInput) {
    int fibInt = 1;
    int previousFibInt = 1;
    if (userInput <= 1) {
        return userInput;
    }

    while (fibInt < userInput) {
        int temp = fibInt;
        fibInt += previousFibInt;
        previousFibInt = temp;
    }
    if (difference(fibInt, userInput) > difference(previousFibInt, userInput)) {
        return previousFibInt;
    } else if (difference(fibInt, userInput) < difference(previousFibInt, userInput)) {
        return fibInt;
    } else {
        return fibInt;
    }    
}
int main() {
    int userInput;
    std::cin >> userInput;
    std::cout << fib(userInput) << std::endl;
    return 0;
}