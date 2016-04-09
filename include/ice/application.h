#include <ice/filesystem/path.h>
#include <system_error>

namespace ice {
namespace application {

// Path to the application filename.
ice::filesystem::path file();

// Path to the application filename.
ice::filesystem::path file(std::error_code& ec);

// Path to the application directory.
ice::filesystem::path path();

// Path to the application directory.
ice::filesystem::path path(std::error_code& ec);

}  // namespace application
}  // namespace ice