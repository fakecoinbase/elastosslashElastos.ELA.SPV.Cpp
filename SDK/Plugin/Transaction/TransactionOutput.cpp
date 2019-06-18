// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionOutput.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/WalletCore/BIPs/Key.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/Payload/OutputPayload/PayloadDefault.h>
#include <SDK/Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>

#include <iostream>
#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		TransactionOutput::TransactionOutput() :
				_outputLock(0),
				_outputType(Type::Default) {
			_amount.setWord(0);
			_payload = GeneratePayload(_outputType);
		}

		TransactionOutput::TransactionOutput(const TransactionOutput &output) {
			this->operator=(output);
		}

		TransactionOutput &TransactionOutput::operator=(const TransactionOutput &o) {
			_amount = o._amount;
			_assetID = o._assetID;
			_programHash = o._programHash;
			_outputLock = o._outputLock;
			_outputType = o._outputType;
			_payload = GeneratePayload(o._outputType);
			*_payload = *o._payload;
			return *this;
		}

		TransactionOutput::TransactionOutput(const BigInt &a, const Address &addr, const uint256 &assetID,
											 Type type, const OutputPayloadPtr &payload) :
			_outputLock(0),
			_outputType(type) {

			_assetID = assetID;
			_amount = a;
			_programHash = addr.ProgramHash();

			if (payload == nullptr) {
				_payload = GeneratePayload(_outputType);
			} else {
				_payload = payload;
			}
		}

		TransactionOutput::TransactionOutput(const BigInt &a, const uint168 &programHash, const uint256 &assetID,
											 Type type, const OutputPayloadPtr &payload) :
			_outputLock(0),
			_outputType(type) {

			_assetID = assetID;
			_amount = a;
			_programHash = programHash;

			if (payload == nullptr) {
				_payload = GeneratePayload(_outputType);
			} else {
				_payload = payload;
			}
		}

		TransactionOutput::~TransactionOutput() {
		}

		Address TransactionOutput::GetAddress() const {
			return Address(_programHash);
		}

		const BigInt &TransactionOutput::GetAmount() const {
			return _amount;
		}

		void TransactionOutput::SetAmount(const BigInt &a) {
			_amount = a;
		}

		size_t TransactionOutput::EstimateSize() const {
			size_t size = 0;
			ByteStream stream;

			size += _assetID.size();
			if (_assetID == Asset::GetELAAssetID()) {
				size += sizeof(uint64_t);
			} else {
				bytes_t amountBytes = _amount.getHexBytes();
				size += stream.WriteVarUint(amountBytes.size());
				size += amountBytes.size();
			}

			size += sizeof(_outputLock);
			size += _programHash.size();

			return size;
		}

		void TransactionOutput::Serialize(ByteStream &ostream) const {
			ostream.WriteBytes(_assetID);

			if (_assetID == Asset::GetELAAssetID()) {
				uint64_t amount = _amount.getWord();
				ostream.WriteUint64(amount);
			} else {
				ostream.WriteVarBytes(_amount.getHexBytes());
			}

			ostream.WriteUint32(_outputLock);
			ostream.WriteBytes(_programHash);
		}

		bool TransactionOutput::Deserialize(const ByteStream &istream) {
			if (!istream.ReadBytes(_assetID)) {
				Log::error("deserialize output assetid error");
				return false;
			}

			if (_assetID == Asset::GetELAAssetID()) {
				uint64_t amount;
				if (!istream.ReadUint64(amount)) {
					Log::error("deserialize output amount error");
					return false;
				}
				_amount.setWord(amount);
			} else {
				bytes_t bytes;
				if (!istream.ReadVarBytes(bytes)) {
					Log::error("deserialize output BN amount error");
					return false;
				}
				_amount.setHexBytes(bytes);
			}

			if (!istream.ReadUint32(_outputLock)) {
				Log::error("deserialize output lock error");
				return false;
			}

			if (!istream.ReadBytes(_programHash)) {
				Log::error("deserialize output program hash error");
				return false;
			}

			return true;
		}

		void TransactionOutput::Serialize(ByteStream &ostream, uint8_t txVersion) const {
			Serialize(ostream);

			if (txVersion >= Transaction::TxVersion::V09) {
				ostream.WriteUint8(_outputType);
				_payload->Serialize(ostream);
			}
		}

		bool TransactionOutput::Deserialize(const ByteStream &istream, uint8_t txVersion) {
			if (!Deserialize(istream)) {
				Log::error("tx output deserialize default part error");
				return false;
			}

			if (txVersion >= Transaction::TxVersion::V09) {
				uint8_t outputType = 0;
				if (!istream.ReadUint8(outputType)) {
					Log::error("tx output deserialize output type error");
					return false;
				}
				_outputType = static_cast<Type>(outputType);

				_payload = GeneratePayload(_outputType);

				if (!_payload->Deserialize(istream)) {
					Log::error("tx output deserialize payload error");
					return false;
				}
			}

			return true;
		}

		bool TransactionOutput::IsValid() const {
			return true;
		}

		const uint256 &TransactionOutput::GetAssetID() const {
			return _assetID;
		}

		void TransactionOutput::SetAssetID(const uint256 &assetId) {
			_assetID = assetId;
		}

		uint32_t TransactionOutput::GetOutputLock() const {
			return _outputLock;
		}

		void TransactionOutput::SetOutputLock(uint32_t lock) {
			_outputLock = lock;
		}

		const uint168 &TransactionOutput::GetProgramHash() const {
			return _programHash;
		}

		void TransactionOutput::SetProgramHash(const uint168 &hash) {
			_programHash = hash;
		}

		const TransactionOutput::Type &TransactionOutput::GetType() const {
			return _outputType;
		}

		void TransactionOutput::SetType(const Type &type) {
			_outputType = type;
		}

		const OutputPayloadPtr &TransactionOutput::GetPayload() const {
			return _payload;
		}

		OutputPayloadPtr &TransactionOutput::GetPayload() {
			return _payload;
		}

		void TransactionOutput::SetPayload(const OutputPayloadPtr &payload) {
			_payload = payload;
		}

		OutputPayloadPtr TransactionOutput::GeneratePayload(const Type &type) {
			OutputPayloadPtr payload;

			switch (type) {
				case Default:
					payload = OutputPayloadPtr(new PayloadDefault());
					break;
				case VoteOutput:
					payload = OutputPayloadPtr(new PayloadVote());
					break;

				default:
					payload = nullptr;
					break;
			}

			return payload;
		}

		nlohmann::json TransactionOutput::ToJson() const {
			nlohmann::json j;

			j["Amount"] = _amount.getDec();
			j["AssetId"] = _assetID.GetHex();
			j["OutputLock"] = _outputLock;
			j["ProgramHash"] = _programHash.GetHex();
			j["Address"] = Address(_programHash).String();

			return j;
		}

		void TransactionOutput::FromJson(const nlohmann::json &j) {

			if (j["Amount"].is_number()) {
				_amount.setWord(j["Amount"].get<uint64_t>());
			} else if (j["Amount"].is_string()) {
				_amount.setDec(j["Amount"].get<std::string>());
			}
			_assetID.SetHex(j["AssetId"].get<std::string>());
			_outputLock = j["OutputLock"].get<uint32_t>();
			_programHash.SetHex(j["ProgramHash"].get<std::string>());
		}

		nlohmann::json TransactionOutput::ToJson(uint8_t txVersion) const {
			nlohmann::json j = ToJson();

			if (txVersion >= Transaction::TxVersion::V09) {
				j["OutputType"] = _outputType;
				j["Payload"] = _payload->ToJson();
			}

			return j;
		}

		void TransactionOutput::FromJson(const nlohmann::json &j, uint8_t txVersion) {
			FromJson(j);

			if (txVersion >= Transaction::TxVersion::V09) {
				_outputType = j["OutputType"];
				_payload = GeneratePayload(_outputType);
				_payload->FromJson(j["Payload"]);
			}
		}

		size_t TransactionOutput::GetSize() const {
			return _assetID.size() + sizeof(_amount) + sizeof(_outputLock) + _programHash.size();
		}

	}
}
