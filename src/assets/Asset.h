#pragma once

#include "common/Common.h"
#include "common/Path.h"
#include "common/Rc.h"
#include <unordered_map>
#include <memory>
#include <cassert>

namespace chisel
{
    struct Asset;
    using AssetID = uint;
    static constexpr AssetID InvalidAssetID = 0;

    struct Asset : public RcObject
    {
        using AssetTable = std::unordered_map<fs::Path, Asset*>;

        AssetID id = ++s_NextID;

        Asset(const fs::Path& path = "")
        {
            if (!path.empty())
            {
                auto res = AssetDB.insert(std::make_pair<fs::Path, Asset*>(fs::Path(path), this));
                assert(res.second);
                m_path = &res.first->first;
            }
        }

        virtual ~Asset()
        {
            if (m_path)
                AssetDB.erase(*m_path);
        }

        const fs::Path& GetPath() const
        {
            return *m_path;
        }

    private:
        const fs::Path* m_path = nullptr;

        friend struct Assets;

        static inline AssetTable AssetDB;
        static inline AssetID s_NextID = 0;

        static inline const fs::Path nullPath = fs::Path();
    };
}

