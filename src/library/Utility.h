#ifndef UTILITY_H
#define UTILITY_H

#include <vector>
using std::vector;


namespace tstudio
{
    template <typename T>
    vector<T> reorder_vector(vector<T> vec1, int oldIndex, int newIndex){
        vector<T> vec2;
        for (int i = 0; i < vec1.size(); i++)
        {
            if (i == newIndex)
            {
                if (newIndex > oldIndex)
                {
                    vec2.push_back(vec1[newIndex]);
                    vec2.push_back(vec1[oldIndex]);
                }
                else if (newIndex < oldIndex)
                {
                    vec2.push_back(vec1[oldIndex]);
                    vec2.push_back(vec1[newIndex]);
                }
                else
                {
                    vec2.push_back(vec1[i]);
                }
            }
            else
            {
                vec2.push_back(vec1[i]);
            }
        }
        vec1.swap(vec2);
        return vec1;
    }
}

#endif // !UTILITY_H