#ifndef UTILITY_H
#define UTILITY_H

#include <vector>
using std::vector;


namespace tstudio
{
    template <typename T>
    void reorder_vector(vector<T> &vec, int oldIndex, int newIndex){
        // auto first = vec1.begin() + oldIndex;
        // auto last = vec1.begin() + oldIndex + 1;
        // auto pos = vec1.begin() + newIndex;
        // if (pos < first)
        //     std::rotate(pos, first, last);
        // if (last < pos)
        //     std::rotate(first, last, pos);

        
        // Move the element using rotate
        if (oldIndex < newIndex)
        {
            std::rotate(vec.begin() + oldIndex, vec.begin() + oldIndex + 1, vec.begin() + newIndex + 1);
        }
        else
        {
            std::rotate(vec.begin() + newIndex, vec.begin() + oldIndex, vec.begin() + oldIndex + 1);
        }
    }
}

#endif // !UTILITY_H