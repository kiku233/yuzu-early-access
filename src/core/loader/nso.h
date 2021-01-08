// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <optional>
#include <type_traits>
#include "common/common_types.h"
#include "common/swap.h"
#include "core/file_sys/patch_manager.h"
#include "core/loader/loader.h"

namespace Core {
class System;
}

namespace Kernel {
class Process;
}

namespace Loader {

struct NSOSegmentHeader {
    u32_le offset;
    u32_le location;
    u32_le size;
    union {
        u32_le alignment;
        u32_le bss_size;
    };
};
static_assert(sizeof(NSOSegmentHeader) == 0x10, "NsoSegmentHeader has incorrect size.");

struct NSOHeader {
    using SHA256Hash = std::array<u8, 0x20>;

    struct RODataRelativeExtent {
        u32_le data_offset;
        u32_le size;
    };

    u32_le magic;
    u32_le version;
    u32 reserved;
    u32_le flags;
    std::array<NSOSegmentHeader, 3> segments; // Text, RoData, Data (in that order)
    std::array<u8, 0x20> build_id;
    std::array<u32_le, 3> segments_compressed_size;
    std::array<u8, 0x1C> padding;
    RODataRelativeExtent api_info_extent;
    RODataRelativeExtent dynstr_extent;
    RODataRelativeExtent dynsyn_extent;
    std::array<SHA256Hash, 3> segment_hashes;

    bool IsSegmentCompressed(size_t segment_num) const;
};
static_assert(sizeof(NSOHeader) == 0x100, "NSOHeader has incorrect size.");
static_assert(std::is_trivially_copyable_v<NSOHeader>, "NSOHeader must be trivially copyable.");

constexpr u32 NSO_ARGUMENT_DATA_ALLOCATION_SIZE = 0x9000;

struct NSOArgumentHeader {
    u32_le allocated_size;
    u32_le actual_size;
    INSERT_PADDING_BYTES(0x18);
};
static_assert(sizeof(NSOArgumentHeader) == 0x20, "NSOArgumentHeader has incorrect size.");

/// Loads an NSO file
class AppLoader_NSO final : public AppLoader {
public:
    explicit AppLoader_NSO(FileSys::VirtualFile file);

    /**
     * Returns the type of the file
     * @param file std::shared_ptr<VfsFile> open file
     * @return FileType found, or FileType::Error if this loader doesn't know it
     */
    static FileType IdentifyType(const FileSys::VirtualFile& file);

    FileType GetFileType() const override {
        return IdentifyType(file);
    }

    static std::optional<VAddr> LoadModule(Kernel::Process& process, Core::System& system,
                                           const FileSys::VfsFile& file, VAddr load_base,
                                           bool should_pass_arguments, bool load_into_process,
                                           std::optional<FileSys::PatchManager> pm = {});

    LoadResult Load(Kernel::Process& process, Core::System& system) override;

    ResultStatus ReadNSOModules(Modules& modules) override;

private:
    Modules modules;
};

} // namespace Loader
