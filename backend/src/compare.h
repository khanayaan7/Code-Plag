#pragma once

#include "winnow.h"

#include <cstddef>
#include <utility>
#include <vector>

namespace codeplag {

using LineRange = std::pair<int, int>;

struct CompareResult {
    double similarity;
    std::vector<LineRange> matchedLines1;
    std::vector<LineRange> matchedLines2;
    std::size_t totalFingerprints1;
    std::size_t totalFingerprints2;
    std::size_t matchedFingerprints;
};

CompareResult compareFingerprints(
    const std::vector<Fingerprint>& fingerprints1,
    const std::vector<Fingerprint>& fingerprints2);

CompareResult compareCode(const std::string& code1, const std::string& code2);

}  // namespace codeplag
