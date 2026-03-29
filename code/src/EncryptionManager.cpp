#include "EncryptionManager.h"
#include <cstring>

using namespace std;

// Constructor
EncryptionManager::EncryptionManager(const string& encryptionKey, bool enabled)
    : key(encryptionKey), encryptionEnabled(enabled) {

    if (key.empty()) {
        key = "ext2sim_default_key";
    }

    // cout << "[EncryptionManager] Initialized. Encryption: "
    //      << (encryptionEnabled ? "ENABLED" : "DISABLED")
    //      << ", Key length: " << key.length() << " bytes" << endl;
}

// Destructor
EncryptionManager::~EncryptionManager() {
    // cout << "[EncryptionManager] Destroyed" << endl;
}

// Encrypt buffer in-place using XOR cipher.
// Each byte is XOR'd with a cycling byte from the key.
// XOR is symmetric: calling encrypt() twice returns the original data.
void EncryptionManager::encrypt(char* buffer, unsigned int length) const {
    if (!encryptionEnabled || key.empty() || buffer == nullptr || length == 0) {
        return;
    }

    unsigned int keyLen = (unsigned int)key.length();

    for (unsigned int i = 0; i < length; i++) {
        buffer[i] ^= key[i % keyLen];
    }
}

// Decrypt is identical to encrypt for XOR cipher (symmetric)
void EncryptionManager::decrypt(char* buffer, unsigned int length) const {
    encrypt(buffer, length);  // XOR is its own inverse
}

// Enable or disable encryption at runtime
void EncryptionManager::setEnabled(bool enabled) {
    encryptionEnabled = enabled;
    // cout << "[EncryptionManager] Encryption "
    //      << (encryptionEnabled ? "ENABLED" : "DISABLED") << endl;
}

bool EncryptionManager::isEnabled() const {
    return encryptionEnabled;
}

// Get the current key
string EncryptionManager::getKey() const {
    return key;
}

// Change the key (NOTE: existing encrypted data will become unreadable
// unless decrypted with the old key first)
void EncryptionManager::setKey(const string& newKey) {
    if (!newKey.empty()) {
        key = newKey;
        cout << "[EncryptionManager] Key updated." << endl;
    }
}
