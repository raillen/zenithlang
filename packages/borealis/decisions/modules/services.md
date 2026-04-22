# Borealis Module Decision - Services

- Status: proposed
- Date: 2026-04-22
- Type: module / network and remote services
- Scope: `borealis.game.services`

## Summary

Services covers network play, cloud save, API calls, matchmaking, and remote sessions.

## Implementation

- keep remote concerns separate from local save and storage;
- keep the API simple for beginners;
- allow future subareas without renaming the module.

## Proposed API

- `services_connect(config)`: opens a remote service connection.
- `services_disconnect(handle)`: closes the connection.
- `services_request(method, url, payload) -> Response`: performs an API request.
- `services_send(handle, message)`: sends a message.
- `services_receive(handle) -> Message`: receives a message.
- `services_host(config) -> Session`: starts a hosted session.
- `services_join(code) -> Session`: joins a session.
- `services_leave(session)`: leaves a session.
- `services_upload(name, data)`: uploads cloud data.
- `services_download(name) -> Data`: downloads cloud data.
- `services_matchmake(config) -> Match`: finds a match.

## Notes

- this is the umbrella module for remote capabilities.
- subareas can be grouped later without changing the main name.
- services can build on stdlib networking and request helpers under the hood.
