#===== Download node modules =====
set(npm_ok_file
  node_modules_installed_successfully
)

add_custom_command(
  OUTPUT ${npm_ok_file}
  COMMAND npm install
  COMMAND echo > ${CMAKE_CURRENT_BINARY_DIR}/${npm_ok_file}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

#===== Build JS bundle =====
set(bundle_files
  yogi.js
  yogi.js.map
)

file(GLOB_RECURSE src_files src/*.js)

add_custom_command(
  OUTPUT ${bundle_files}
  COMMAND npm run build -- --output-path=${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${src_files} ${npm_ok_file}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target(yogi-javascript
  ALL
  DEPENDS ${bundle_files}
)

add_test(
  NAME yogi-javascript
  COMMAND npm test
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
