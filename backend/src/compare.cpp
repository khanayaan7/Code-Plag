#include "compare.h"

#include "hashing.h"
#include "tokenizer.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace codeplag {
namespace {

std::vector<LineRange> mergeRanges(std::vector<LineRange> ranges) {
    if (ranges.empty()) {
        return {};
    }

    std::sort(ranges.begin(), ranges.end());
    std::vector<LineRange> merged;
    merged.push_back(ranges.front());

    for (std::size_t index = 1; index < ranges.size(); ++index) {
        auto& current = merged.back();
        const auto& next = ranges[index];
        if (next.first <= current.second + 1) {
            current.second = std::max(current.second, next.second);
        } else {
            merged.push_back(next);
        }
    }

    return merged;
}

std::unordered_set<std::uint64_t> hashSet(
    const std::vector<Fingerprint>& fingerprints) {
    std::unordered_set<std::uint64_t> hashes;
    hashes.reserve(fingerprints.size());
    for (const auto& fingerprint : fingerprints) {
        hashes.insert(fingerprint.hash);
    }
    return hashes;
}

std::vector<LineRange> matchedRanges(
    const std::vector<Fingerprint>& fingerprints,
    const std::unordered_set<std::uint64_t>& otherHashes) {
    std::vector<LineRange> ranges;
    for (const auto& fingerprint : fingerprints) {
        if (otherHashes.count(fingerprint.hash) != 0) {
            ranges.emplace_back(fingerprint.lineStart, fingerprint.lineEnd);
        }
    }
    return mergeRanges(std::move(ranges));
}

}  // namespace

CompareResult compareFingerprints(
    const std::vector<Fingerprint>& fingerprints1,
    const std::vector<Fingerprint>& fingerprints2) {
    CompareResult result{
        0.0,
        {},
        {},
        fingerprints1.size(),
        fingerprints2.size(),
        0,
    };

    if (fingerprints1.empty() || fingerprints2.empty()) {
        return result;
    }

    const auto hashes1 = hashSet(fingerprints1);
    const auto hashes2 = hashSet(fingerprints2);

    for (const auto hash : hashes1) {
        if (hashes2.count(hash) != 0) {
            ++result.matchedFingerprints;
        }
    }

    const std::size_t smallerFingerprintCount =
        std::min(hashes1.size(), hashes2.size());

    // Containment is intentional: unlike Jaccard similarity, it does not punish a
    // copied short submission merely because it was embedded in a much longer one.
    // Both numerator and denominator use unique hashes. Mixing a set-intersection
    // numerator with a raw-vector denominator would make identical repetitive code
    // score below 100% because the same selected hash can occur at multiple lines.
    result.similarity =
        static_cast<double>(result.matchedFingerprints) /
        static_cast<double>(smallerFingerprintCount) * 100.0;
    result.matchedLines1 = matchedRanges(fingerprints1, hashes2);
    result.matchedLines2 = matchedRanges(fingerprints2, hashes1);

    return result;
}

CompareResult compareCode(const std::string& code1, const std::string& code2) {
    const auto fingerprints1 = winnow(hashKGrams(tokenize(code1)));
    const auto fingerprints2 = winnow(hashKGrams(tokenize(code2)));
    return compareFingerprints(fingerprints1, fingerprints2);
}

}  // namespace codeplag
