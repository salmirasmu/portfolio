#include <iostream>
#include <vector>

using namespace std;

int binarySearch(std::vector<int> numbers, int left, int right, int value)
{
    while (left <= right)
    {
        int middle = (left+right)/2;
        if (numbers.at(middle) < value)
        {
            left = middle+1;
        }
        else if (numbers.at(middle) > value)
        {
            right = middle-1;
        }
        else
        {
            return middle;
        }
    }
    return -1;
}

int main()
{
    std::vector<int> n = {1,2,3,5,6,7,8,9,10,11};
    std::vector<int> m = {2};
    int i = binarySearch(n,0,n.size()-1,4);
    std::cout << i << endl;
    return 0;
}


