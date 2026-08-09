# 026-08-09:
in Mod.hpp process_manifest should edit a list of files that it processed to enable feedback.
Also process_manifest in activate does not know which transfer it is: move to backup or copy to game,
 so there is need to create two manifests from one file list.
