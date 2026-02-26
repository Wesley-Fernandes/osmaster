import { createRequire } from "module";
const require = createRequire(import.meta.url);
const native = require("./build/Release/osmaster_native.node");

console.log(native.hello());
