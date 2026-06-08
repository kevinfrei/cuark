import {
  arr,
  bool,
  dbl,
  Enum,
  enum_lst,
  enum_num,
  enum_str,
  map,
  NEnum,
  num,
  obj,
  opt,
  ref,
  SEnum,
  str,
  Types,
  u16,
  u64,
  u8,
} from 'crow-idl/IDL';

const CurrentView: NEnum = enum_num(num(), {
  disabled: -1,
  none: 0,
  settings: 1,
  tools: 2,
  search: 3,
  // Additional entries after this one:
  __last_cuark_view: 100,
});

const StrId: SEnum = enum_str({
  FilePath: 'File Path',
  FilesSelected: 'Files Selected',
  ErrNotSingleAndNotMultiple: 'Not Single and not Multiple (This is a bug!)',
  ErrSingleAndMultiple: 'Both Single and Multiple (This is a bug!)',
  ViewSettings: 'Settings',
  ViewTools: 'Tools',
  // Other strings after this one:
  __last_cuark_StrId: '',
});

const Keys: SEnum = enum_str({
  Find: 'F',
  Settings: ',',
  Next: 'Right',
  Prev: 'Left',
  Tools: 'L',
  // Other Keys after this one:
  __last_cuark_Key: '',
});

const IpcCall: NEnum = enum_num(u16(), {
  Unknown: 0,
  ReadFromStorage: 1,
  WriteToStorage: 2,
  DeleteFromStorage: 3,
  MinimizeWindow: 4,
  MaximizeWindow: 5,
  RestoreWindow: 6,
  CloseWindow: 7,
  IsDev: 8,
  AsyncData: 9,
  MenuAction: 10,
  ShowOpenDialog: 11,
  GetFileSystemRoots: 12,
  GetNamedLocations: 13,
  GetFolderContents: 14,
  // Other IpcCall's after this one:
  __last_cuark_IpcCall: 1000,
});

const SocketMsg: Enum = enum_lst(u8(), [
  'Unknown',
  'ContentLoaded',
  'KeepAlive',
  // Other SocketMsg strings after this one:
  '__last_cuark_SocketMsg',
]);

const StorageId: SEnum = enum_str({
  CurrentView: 'currentView',
  SettingValue: 'someSetting',
  // Other StorageID's after this one:
  __last_cuark_StorageId: '',
});

const MimeData = obj({
  type: str(),
  data: str(),
});

const FileFilterItem = obj({ name: str(), extensions: arr(str()) });

const OpenDialogOptions = obj({
  folder: opt(bool()),
  title: opt(str()),
  defaultPath: opt(str()),
  buttonLabel: opt(str()),
  multiSelections: opt(bool()),
  filters: opt(arr(ref('FileFilterItem'))),
});

const NamedLocations = map(str(), str());

const FileSystemItem = obj({
  file: str(),
  date: dbl(),
  size: u64(),
  type: str(),
});

const FolderContents = arr(ref('FileSystemItem'));

// Additional types go *below* this point:

// Additional types go *above* this point:

export const TypesToGenerate: Record<string, Types> = {
  Keys,
  StrId,
  CurrentView,
  IpcCall,
  SocketMsg,
  StorageId,
  MimeData,
  FileFilterItem,
  OpenDialogOptions,
  NamedLocations,
  FileSystemItem,
  FolderContents,
  // Other TypesToGenerate below this comment:
};

export const PicklersToGenerate: Record<string, Types> = {};
