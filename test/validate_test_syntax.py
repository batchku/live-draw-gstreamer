#!/usr/bin/env python3
"""
Test Syntax Validator
Validates that test files have proper structure and syntax
"""

import re
import sys
from pathlib import Path

class TestValidator:
    def __init__(self):
        self.test_dir = Path("test/unit")
        self.errors = []
        self.warnings = []

    def validate_file(self, filepath, module_name):
        """Validate a test file"""
        print(f"\nValidating: {module_name}")
        print("-" * 50)

        if not filepath.exists():
            self.errors.append(f"{filepath}: File not found")
            return False

        with open(filepath, 'r') as f:
            content = f.read()

        # Check for test functions
        test_patterns = [
            r'static\s+(?:int|gboolean|void)\s+test_\w+\s*\((?:void)?\)',
        ]

        test_count = 0
        for pattern in test_patterns:
            matches = re.findall(pattern, content)
            test_count += len(matches)
            if test_count == 0:
                # Try simpler pattern
                simple_pattern = r'static\s+(?:int|gboolean|void)\s+test_'
                test_count = len(re.findall(simple_pattern, content))

        if test_count == 0:
            self.errors.append(f"{filepath}: No test functions found")
            return False

        print(f"✓ Found {test_count} test functions")

        # Check for main function
        if 'int main' not in content:
            self.warnings.append(f"{filepath}: No main function found")
        else:
            print("✓ Main function exists")

        # Check for assertions
        assertion_count = 0
        assertion_patterns = ['assert(', 'ASSERT_', 'fprintf(stderr']
        for pattern in assertion_patterns:
            assertion_count += content.count(pattern)

        if assertion_count == 0:
            self.warnings.append(f"{filepath}: No assertions found")
        else:
            print(f"✓ Found {assertion_count} assertions/checks")

        # Check for includes
        has_includes = 'include' in content
        if not has_includes:
            self.warnings.append(f"{filepath}: No includes found")
        else:
            print("✓ Has necessary includes")

        # Check for proper syntax - matching braces
        opening_braces = content.count('{')
        closing_braces = content.count('}')
        if opening_braces != closing_braces:
            self.errors.append(f"{filepath}: Brace mismatch ({opening_braces} open, {closing_braces} close)")
            return False

        print("✓ Brace syntax OK")

        # Basic C syntax checks
        if '/*' in content and '*/' not in content:
            self.warnings.append(f"{filepath}: Unclosed comment block")

        return True

    def validate_all(self):
        """Validate all test files"""
        tests_to_validate = [
            ("Recording State", "test_recording_state.c"),
            ("Buffer Manager", "test_buffer_manager.c"),
            ("Playback Manager", "test_playback_manager.c"),
            ("Keyboard Handler", "test_keyboard_handler.c"),
        ]

        print("=" * 50)
        print("TEST SYNTAX VALIDATION")
        print("=" * 50)

        all_valid = True
        for module_name, filename in tests_to_validate:
            filepath = self.test_dir / filename
            if not self.validate_file(filepath, module_name):
                all_valid = False

        # Print summary
        print("\n" + "=" * 50)
        print("VALIDATION SUMMARY")
        print("=" * 50)

        if self.errors:
            print(f"\n✗ ERRORS ({len(self.errors)}):")
            for error in self.errors:
                print(f"  - {error}")
            all_valid = False

        if self.warnings:
            print(f"\n⚠ WARNINGS ({len(self.warnings)}):")
            for warning in self.warnings:
                print(f"  - {warning}")

        if not self.errors:
            print("\n✓ All tests have valid syntax")

        print("\n" + "=" * 50)
        return 0 if all_valid else 1

def main():
    validator = TestValidator()
    return validator.validate_all()

if __name__ == "__main__":
    sys.exit(main())
