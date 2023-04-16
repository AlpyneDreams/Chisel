#pragma once

namespace chisel
{
    bool ExportBox(std::string_view filepath, Map& map);
    bool ExportMap(std::string_view filepath, Map& map);
    bool ExportVMF(std::string_view filepath, Map& map);

    bool ImportBox(std::string_view filepath, Map& map);
    bool ImportVMF(std::string_view filepath, Map& map);
}
