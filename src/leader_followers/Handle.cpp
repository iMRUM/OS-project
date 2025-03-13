#include "../../include/leader_followers/Handle.hpp"


bool Handle::close() {
    if (!isValid()) {
        return false; // Already closed or invalid
    }
    
    int result = ::close(handle_);
    
    if (result == 0) {
        // Successfully closed
        handle_ = -1;
        return true;
    } else {
        // Failed to close
        std::cerr << "Error closing handle " << handle_ << ": " 
                  << strerror(errno) << std::endl;
        return false;
    }
}

std::string Handle::toString() const {
    std::stringstream ss;
    ss << "Handle(" << handle_ << ")";
    return ss.str();
}