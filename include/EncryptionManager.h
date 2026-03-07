#ifndef ENCRYPTIONMANAGER_H
#define ENCRYPTIONMANAGER_H

#include <string>
#include <cstddef>
#include <iostream>

// EncryptionManager - Provides XOR-based symmetric encryption for file data.
// Only file DATA blocks are encrypted; metadata (inodes, bitmaps, superblock,
// directory entries) remain unencrypted so the FS engine can read them normally.
class EncryptionManager {
private:
    std::string key;           // Encryption key (XOR key bytes cycle)
    bool encryptionEnabled;    // Toggle encryption on/off

public:
    // Constructor - provide any non-empty key string
    EncryptionManager(const std::string& encryptionKey, bool enabled = true);

    // Destructor
    ~EncryptionManager();

    // Encrypt a buffer in-place
    void encrypt(char* buffer, unsigned int length) const;

    // Decrypt a buffer in-place (XOR is symmetric, same operation)
    void decrypt(char* buffer, unsigned int length) const;

    // Enable / disable encryption at runtime
    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Get the current key
    std::string getKey() const;

    // Change the key
    void setKey(const std::string& newKey);
};

#endif // ENCRYPTIONMANAGER_H
