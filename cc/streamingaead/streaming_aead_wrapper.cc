// Copyright 2019 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#include "tink/streamingaead/streaming_aead_wrapper.h"

#include "tink/streaming_aead.h"
#include "tink/crypto_format.h"
#include "tink/input_stream.h"
#include "tink/output_stream.h"
#include "tink/primitive_set.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"

namespace crypto {
namespace tink {

using ::crypto::tink::util::Status;
using ::crypto::tink::util::StatusOr;

namespace {

Status Validate(PrimitiveSet<StreamingAead>* primitives) {
  if (primitives == nullptr) {
    return Status(util::error::INTERNAL, "primitive set must be non-NULL");
  }
  if (primitives->get_primary() == nullptr) {
    return Status(util::error::INVALID_ARGUMENT,
                        "primitive set has no primary");
  }
  return Status::OK;
}

class StreamingAeadSetWrapper: public StreamingAead {
 public:
  explicit StreamingAeadSetWrapper(
      std::unique_ptr<PrimitiveSet<StreamingAead>> primitives)
      : primitives_(std::move(primitives)) {}

  crypto::tink::util::StatusOr<std::unique_ptr<crypto::tink::OutputStream>>
  NewEncryptingStream(
      std::unique_ptr<crypto::tink::OutputStream> ciphertext_destination,
      absl::string_view associated_data) override;

  crypto::tink::util::StatusOr<std::unique_ptr<crypto::tink::InputStream>>
  NewDecryptingStream(
      std::unique_ptr<crypto::tink::InputStream> ciphertext_source,
      absl::string_view associated_data) override;

  ~StreamingAeadSetWrapper() override {}

 private:
  std::unique_ptr<PrimitiveSet<StreamingAead>> primitives_;
};  // class StreamingAeadSetWrapper

StatusOr<std::unique_ptr<OutputStream>>
StreamingAeadSetWrapper::NewEncryptingStream(
    std::unique_ptr<OutputStream> ciphertext_destination,
    absl::string_view associated_data) {
  return primitives_->get_primary()->get_primitive().NewEncryptingStream(
          std::move(ciphertext_destination), associated_data);
}

StatusOr<std::unique_ptr<InputStream>>
StreamingAeadSetWrapper::NewDecryptingStream(
    std::unique_ptr<InputStream> ciphertext_source,
    absl::string_view associated_data) {
  return Status(util::error::UNIMPLEMENTED, "Not implemented yet");
}

}  // anonymous namespace

StatusOr<std::unique_ptr<StreamingAead>> StreamingAeadWrapper::Wrap(
    std::unique_ptr<PrimitiveSet<StreamingAead>> streaming_aead_set) const {
  auto status = Validate(streaming_aead_set.get());
  if (!status.ok()) return status;
  std::unique_ptr<StreamingAead> streaming_aead =
      absl::make_unique<StreamingAeadSetWrapper>(
          std::move(streaming_aead_set));
  return std::move(streaming_aead);
}

}  // namespace tink
}  // namespace crypto
