// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadTransferAsset.h"

namespace Elastos {
	namespace SDK {

		PayloadTransferAsset::PayloadTransferAsset() {

		}

		PayloadTransferAsset::~PayloadTransferAsset() {

		}

		ByteData PayloadTransferAsset::getData() const {
			//todo implement IPayload getData
			return ByteData(nullptr, 0);
		}

		void PayloadTransferAsset::Serialize(ByteStream &ostream) const {

		}

		void PayloadTransferAsset::Deserialize(ByteStream &istream) {

		}
	}
}