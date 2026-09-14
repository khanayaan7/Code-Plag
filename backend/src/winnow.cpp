#include "winnow.h"

#include <algorithm>
#include <deque>
#include <limits>

namespace codeplag {

std::vector<Fingerprint> winnow(
    const std::vector<KGramHash>& hashes,
    std::size_t windowSize) {
    if (hashes.empty() || windowSize == 0) {
        return {};
    }

    // A short hash stream still has a useful minimum fingerprint. This is the
    // usual boundary behavior for documents with at least k but fewer than w+k-1
    // tokens; only documents shorter than k produce no fingerprints.
    const std::size_t effectiveWindowSize = std::min(windowSize, hashes.size());
    std::deque<std::size_t> minimumCandidates;
    std::vector<Fingerprint> fingerprints;
    std::size_t lastSelected = std::numeric_limits<std::size_t>::max();

    for (std::size_t index = 0; index < hashes.size(); ++index) {
        while (!minimumCandidates.empty() &&
               hashes[minimumCandidates.back()].hash >= hashes[index].hash) {
            // Removing equal values is what makes the tie break rightmost.
            minimumCandidates.pop_back();
        }
        minimumCandidates.push_back(index);

        if (index + 1 < effectiveWindowSize) {
            continue;
        }

        const std::size_t windowStart = index + 1 - effectiveWindowSize;
        while (!minimumCandidates.empty() &&
               minimumCandidates.front() < windowStart) {
            minimumCandidates.pop_front();
        }

        const std::size_t selected = minimumCandidates.front();
        if (selected != lastSelected) {
            const auto& hash = hashes[selected];
            fingerprints.push_back({hash.hash, hash.lineStart, hash.lineEnd});
            lastSelected = selected;
        }
    }

    return fingerprints;
}

}  // namespace codeplag
