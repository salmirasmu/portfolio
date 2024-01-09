
// Rasmus Salmi

#include <iostream>
#include <vector>

using namespace std;

int partition(std::vector<int>& numbers, int left, int right)
{
    int pivot = numbers[right];
    int cut = left-1;
    for (int i = left; i < right; i++)
    {
        if (numbers[i] <= pivot)
        {
            cut = cut+1;
            std::swap(numbers[i],numbers[cut]);
        }
    }
    std::swap(numbers[cut+1],numbers[right]);
    return cut+1;
}

void quickSort(std::vector<int>& numbers, int left, int right)
{
    if (left < right)
    {
        int pivot = partition(numbers, left, right);
        quickSort(numbers, left, pivot-1);
        quickSort(numbers, pivot+1, right);
    }
}

int main()
{
    std::vector<int> numbers = {1,9,44,8,1,23,78,4,67,2,5,3,7,8,4,6,10};

    for (int i : numbers)
    {
        std::cout<<i<<" ";;
    }
    std::cout<<"\n";
    std::cout<<"\n";

    quickSort(numbers,0,numbers.size()-1);

    for (int i : numbers)
    {
        std::cout<<i<<" ";
    }
    std::cout<<"\n";

    return 0;
}

