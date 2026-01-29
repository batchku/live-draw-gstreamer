# Contributing to Video Looper

Thank you for your interest in contributing to Video Looper! This document provides guidelines for development, code style, and testing practices.

## Getting Started

1. **Clone the repository**
2. **Install dependencies**: See README.md
3. **Build the project**: `./scripts/build.sh`
4. **Run tests**: `./scripts/test.sh`

## Code Style and Standards

### Formatting

- Code must follow C99 standard
- Use `clang-format` for automatic formatting
- All code must compile with `-Wall -Wextra -Wpedantic` without warnings

**Format code before committing**:
```bash
./scripts/format_code.sh
```

### Naming Conventions

- **Variables**: `snake_case` (e.g., `recording_duration`)
- **Functions**: `snake_case` (e.g., `camera_source_init()`)
- **Macros**: `UPPER_SNAKE_CASE` (e.g., `CAMERA_WIDTH`)
- **Types**: `PascalCase` ending in `_t` (e.g., `RecordingState_t`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_RECORDINGS`)

### Code Organization

- **One component per file pair** (.c and .h)
- **Header guards**: `#ifndef FILENAME_H` / `#define FILENAME_H`
- **Include order**: System, GStreamer, local project
- **Type definitions**: In header files only
- **Function documentation**: Doxygen-style comments for public APIs

Example:
```c
/**
 * Initialize recording state manager
 *
 * @return Allocated RecordingState, or NULL on failure
 */
RecordingState* recording_state_init(void);
```

### Error Handling

- Check all allocation results (`malloc`, `calloc`)
- Return NULL or error code for failures
- Log errors using the logging API
- Clean up resources in error paths (use goto cleanup pattern if necessary)

Example:
```c
GstElement* element = gst_element_factory_make("queue", "my-queue");
if (!element) {
  LOG_ERROR("Failed to create queue element");
  return FALSE;
}
```

## Development Workflow

### Building

```bash
# Development build with debug symbols
./scripts/build.sh --debug

# Release build (optimized)
./scripts/build.sh --release

# Clean build
./scripts/build.sh --clean
```

### Testing

**Write tests alongside implementation**. Each component should have corresponding unit and/or integration tests.

```bash
# Run all tests
./scripts/test.sh

# Run specific test suite
./scripts/test.sh --unit
./scripts/test.sh --integration

# Check coverage
./scripts/test.sh --coverage
```

### Profiling

```bash
# Profile CPU usage
./scripts/profile.sh --cpu --duration 60

# Profile memory usage
./scripts/profile.sh --memory

# Run with GStreamer debug output
./scripts/run.sh --debug
```

## Testing Requirements

### Unit Tests

- **Location**: `test/unit/`
- **Coverage target**: 85% of component logic
- **Naming**: `test_<component>.c`

Example:
```c
#include <gtest/gtest.h>
#include "recording/recording_state.h"

TEST(RecordingState, KeyPressTriggersRecording) {
  RecordingState *state = recording_state_init();
  EXPECT_NOT_NULL(state);

  recording_on_key_press(state, 1);
  EXPECT_TRUE(recording_is_recording(state, 1));

  recording_state_cleanup(state);
}
```

### Integration Tests

- **Location**: `test/integration/`
- **Coverage target**: 60% of component interactions
- **Scope**: Test components working together
- **Naming**: `test_<feature>.c`

### Test Execution

All tests must pass before submitting a pull request:
```bash
./scripts/test.sh --build
```

## Documentation

### Code Comments

- Use comments for **why**, not **what**
- Code should be self-documenting through clear naming
- Comment non-obvious algorithms or platform-specific code

Bad:
```c
i++; // increment i
```

Good:
```c
// Advance to next frame in palindrome sequence
current_frame++;
```

### Public APIs

Document all public functions with Doxygen-style comments:

```c
/**
 * Create a new recording buffer
 *
 * @param max_frames Maximum number of frames to store
 * @param caps GStreamer capabilities for the buffer
 * @return Newly allocated RingBuffer, or NULL on failure
 *
 * The buffer is allocated on GPU memory. Caller responsible for
 * calling buffer_cleanup() to free resources.
 */
RingBuffer* buffer_create(guint max_frames, GstCaps *caps);
```

### Architecture Documentation

- Update docs/architecture/ when making significant changes
- Document new components in SDD-referenced files
- Update README.md for user-visible changes

## Performance Considerations

### CPU/GPU Budgets

- **Target CPU**: < 5% of single core for video processing
- **Target FPS**: 120 fps sustained
- **Input latency**: < 50 ms
- **Memory**: < 10% growth/hour

### Profiling Checklist

Before optimizing, **measure first**:
```bash
./scripts/profile.sh --cpu --duration 60
```

Common optimization targets:
1. Queue sizing (max-size-buffers, latency)
2. Buffer allocation patterns
3. GStreamer element properties
4. Synchronization overhead

## Commit Messages

Follow conventional commit format:

```
<type>: <subject>

<body>

<footer>
```

Types: `feat`, `fix`, `refactor`, `test`, `docs`, `chore`

Example:
```
feat: implement palindrome playback algorithm

- Add forward/reverse direction tracking
- Implement frame boundary handling
- Add comprehensive unit tests

Closes #123
```

## Pull Request Process

1. **Create feature branch**: `git checkout -b feature/your-feature`
2. **Implement changes** with tests
3. **Run formatting**: `./scripts/format_code.sh`
4. **Run tests**: `./scripts/test.sh --build`
5. **Update documentation** if needed
6. **Commit with meaningful messages**
7. **Create PR** with description referencing related issues
8. **Address review feedback** with new commits

## Architecture Review Checklist

Before submitting major architectural changes:

- [ ] Design follows SDD patterns
- [ ] Component has clear responsibility (Single Responsibility Principle)
- [ ] Dependencies are minimal and explicit
- [ ] Public API is documented
- [ ] Error handling is comprehensive
- [ ] Tests cover main code paths
- [ ] Performance implications documented

## Common Tasks

### Adding a New Component

1. **Create header file**: `src/<component>/<component>.h`
2. **Create implementation**: `src/<component>/<component>.c`
3. **Add to meson.build** source files list
4. **Create unit tests**: `test/unit/test_<component>.c`
5. **Document public API**: Doxygen comments
6. **Update README.md** if user-visible

### Adding Dependencies

1. **Update meson.build** with new dependency
2. **Document in README.md**
3. **Add to docs/EXTERNAL.md** if external library
4. **Justify addition** (why is it needed?)
5. **Verify build works**: `./scripts/build.sh --clean`

### Performance Optimization

1. **Measure current performance**: `./scripts/profile.sh`
2. **Identify bottleneck**: CPU, GPU, memory, I/O?
3. **Implement optimization** with clear intent
4. **Re-measure**: Verify improvement
5. **Document decision**: Why was this change needed?
6. **Add performance tests**: Prevent regression

## Getting Help

- **Architecture questions**: Review SDD (docs/planning/SDD.md)
- **Requirements questions**: Review PRD (docs/planning/PRD.md)
- **GStreamer questions**: Check official documentation
- **macOS APIs**: Apple Developer Documentation
- **Build issues**: Check README.md troubleshooting

## Code Review Guidelines

When reviewing code:

- Verify tests are present and meaningful
- Check for memory leaks (especially GStreamer resources)
- Ensure error handling is comprehensive
- Validate performance assumptions
- Confirm code style consistency
- Verify documentation is updated

## Release Process

1. **Create release branch**: `git checkout -b release/v1.x.x`
2. **Update version** in meson.build and README.md
3. **Update CHANGELOG.md**
4. **Run full test suite**: `./scripts/test.sh --coverage`
5. **Create release tag**: `git tag v1.x.x`
6. **Push and create release** with notes

## Questions?

Refer to the project documentation:
- **Architecture**: docs/planning/SDD.md
- **Requirements**: docs/planning/PRD.md
- **Implementation**: docs/planning/TTL.md
- **Usage**: README.md

---

Thank you for contributing to Video Looper!
