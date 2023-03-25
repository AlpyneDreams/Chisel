#pragma once

#include <vector>
#include <unordered_map>
#include <string>

namespace chisel
{
    class FGD
    {
        /*enum ClassType
        {
            BaseClass, PointClass, SolidClass, NPCClass, KeyframeClass, MoveClass, FilterClass
        };*/
        using String = std::string;
        template <class T>
        using Dict = std::unordered_map<std::string, T>;
        template <class T>
        using List = std::vector<T>;

        friend struct FGDParser;
    public:
        FGD(const char* path);

        struct Base
        {
            String name;
            String description;
        };

        struct Var : Base
        {
            String type; // TODO: Enum
            String displayName;
            String defaultValue;
            bool report = false;
            bool readOnly = false;
        };

        struct InputOutput : Base
        {
            String type; // TODO: Enum
        };

        struct Helper
        {
            String name;
            List<String> params;
        };

        struct Class : Base
        {
            Dict<Var> variables;
            Dict<InputOutput> inputs;
            Dict<InputOutput> outputs;
            List<Class*> bases;
            List<Helper> helpers;
        };

        List<Class> classes;

        int minSize = -16384;
        int maxSize = 16384;
    };
}
