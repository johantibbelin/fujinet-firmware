#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <stdexcept>
#include "Packet.h"

// Forward reference to break circular dependency
class Response;

class Request : public Packet
{
public:
	Request(uint8_t request_sequence_number, uint8_t command_number, uint8_t sp_unit);
	std::vector<uint8_t> serialize() const override = 0;
	virtual std::unique_ptr<Response> deserialize(const std::vector<uint8_t>& data) const = 0;
	uint8_t get_command_number() const;
	uint8_t get_sp_unit() const;

	// Create the subclass specific Request type from the packet data
	static std::unique_ptr<Request> from_packet(const std::vector<uint8_t>& packet);

	// These are implemented per subclass if they are required.
	virtual void copy_payload(uint8_t* data) const = 0;
	virtual size_t payload_size() const = 0;

	// Creates a Response subclass version specific to the Request subclass, using the data given to us to process
	virtual std::unique_ptr<Response> create_response(uint8_t source, uint8_t status, const uint8_t* data, uint16_t num) const = 0;

	// this is part of the iwm_decoded_cmd_t handling (even though we're given a pointer to its data), clearing the output data and setting the command value in it
	void init_command(uint8_t* cmd_data) const;
	virtual void create_command(uint8_t* cmd_data) const = 0;

private:
	uint8_t command_number_ = 0;
	uint8_t sp_unit_ = 0;
};
