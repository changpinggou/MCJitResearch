clang -x objective-c -arch arm64 -fobjc-arc  -emit-llvm -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk -miphoneos-version-min=9.0 -c bitcode_test.m -o bitcode_test.bc