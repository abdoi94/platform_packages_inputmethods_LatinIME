/*
 * Copyright (C) 2013, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "suggest/policyimpl/dictionary/structure/v4/ver4_patricia_trie_node_reader.h"

#include "suggest/policyimpl/dictionary/structure/v2/patricia_trie_reading_utils.h"
#include "suggest/policyimpl/dictionary/structure/v3/dynamic_patricia_trie_reading_utils.h"
#include "suggest/policyimpl/dictionary/structure/v4/content/probability_dict_content.h"
#include "suggest/policyimpl/dictionary/structure/v4/content/probability_entry.h"
#include "suggest/policyimpl/dictionary/structure/v4/ver4_patricia_trie_reading_utils.h"
#include "suggest/policyimpl/dictionary/utils/buffer_with_extendable_buffer.h"
#include "suggest/policyimpl/dictionary/utils/forgetting_curve_utils.h"

namespace latinime {

const PtNodeParams Ver4PatriciaTrieNodeReader::fetchPtNodeInfoFromBufferAndProcessMovedPtNode(
        const int ptNodePos, const int siblingNodePos) const {
    if (ptNodePos < 0 || ptNodePos >= mBuffer->getTailPosition()) {
        // Reading invalid position because of bug or broken dictionary.
        AKLOGE("Fetching PtNode info from invalid dictionary position: %d, dictionary size: %d",
                ptNodePos, mBuffer->getTailPosition());
        ASSERT(false);
        return PtNodeParams();
    }
    const bool usesAdditionalBuffer = mBuffer->isInAdditionalBuffer(ptNodePos);
    const uint8_t *const dictBuf = mBuffer->getBuffer(usesAdditionalBuffer);
    int pos = ptNodePos;
    const int headPos = ptNodePos;
    if (usesAdditionalBuffer) {
        pos -= mBuffer->getOriginalBufferSize();
    }
    const PatriciaTrieReadingUtils::NodeFlags flags =
            PatriciaTrieReadingUtils::getFlagsAndAdvancePosition(dictBuf, &pos);
    const int parentPosOffset =
            DynamicPatriciaTrieReadingUtils::getParentPtNodePosOffsetAndAdvancePosition(
                    dictBuf, &pos);
    const int parentPos =
            DynamicPatriciaTrieReadingUtils::getParentPtNodePos(parentPosOffset, headPos);
    int codePoints[MAX_WORD_LENGTH];
    const int codePonitCount = PatriciaTrieReadingUtils::getCharsAndAdvancePosition(
            dictBuf, flags, MAX_WORD_LENGTH, codePoints, &pos);
    int terminalIdFieldPos = NOT_A_DICT_POS;
    int terminalId = Ver4DictConstants::NOT_A_TERMINAL_ID;
    int probability = NOT_A_PROBABILITY;
    if (PatriciaTrieReadingUtils::isTerminal(flags)) {
        terminalIdFieldPos = pos;
        if (usesAdditionalBuffer) {
            terminalIdFieldPos += mBuffer->getOriginalBufferSize();
        }
        terminalId = Ver4PatriciaTrieReadingUtils::getTerminalIdAndAdvancePosition(dictBuf, &pos);
        const ProbabilityEntry probabilityEntry =
                mProbabilityDictContent->getProbabilityEntry(terminalId);
        if (probabilityEntry.hasHistoricalInfo()) {
            probability = ForgettingCurveUtils::decodeProbability(
                    probabilityEntry.getHistoricalInfo());
        } else {
            probability = probabilityEntry.getProbability();
        }
    }
    int childrenPosFieldPos = pos;
    if (usesAdditionalBuffer) {
        childrenPosFieldPos += mBuffer->getOriginalBufferSize();
    }
    int childrenPos = DynamicPatriciaTrieReadingUtils::readChildrenPositionAndAdvancePosition(
            dictBuf, &pos);
    if (usesAdditionalBuffer && childrenPos != NOT_A_DICT_POS) {
        childrenPos += mBuffer->getOriginalBufferSize();
    }
    if (usesAdditionalBuffer) {
        pos += mBuffer->getOriginalBufferSize();
    }
    // Sibling position is the tail position of original PtNode.
    int newSiblingNodePos = (siblingNodePos == NOT_A_DICT_POS) ? pos : siblingNodePos;
    // Read destination node if the read node is a moved node.
    if (DynamicPatriciaTrieReadingUtils::isMoved(flags)) {
        // The destination position is stored at the same place as the parent position.
        return fetchPtNodeInfoFromBufferAndProcessMovedPtNode(parentPos, newSiblingNodePos);
    } else {
        return PtNodeParams(headPos, flags, parentPos, codePonitCount, codePoints,
                terminalIdFieldPos, terminalId, probability, childrenPosFieldPos, childrenPos,
                newSiblingNodePos);
    }
}

}