#pragma once

class NonCopyableAndNonMovable {
protected:
  NonCopyableAndNonMovable() = default;
  ~NonCopyableAndNonMovable() = default;

public:
  NonCopyableAndNonMovable(const NonCopyableAndNonMovable&) = delete;
  NonCopyableAndNonMovable& operator=(const NonCopyableAndNonMovable&) = delete;
};
