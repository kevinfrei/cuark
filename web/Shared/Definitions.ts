import {
  arr,
  bool,
  Enum,
  enum_lst,
  enum_num,
  enum_str,
  NEnum,
  num,
  obj,
  opt,
  ref,
  SEnum,
  str,
  Types,
  u8,
} from './IDL';

const CurrentView: NEnum = enum_num(num(), {
  disabled: -1,
  none: 0,
  settings: 1,
  tools: 2,
  search: 3,
});

const StrId: SEnum = enum_str({
  FilePath: 'File Path',
  FilesSelected: 'Files Selected',
  ErrNotSingleAndNotMultiple: 'Not Single and not Multiple (This is a bug!)',
  ErrSingleAndMultiple: 'Both Single and Multiple (This is a bug!)',
  ViewSettings: 'Settings',
  ViewTools: 'Tools',
});

const Keys: SEnum = enum_str({
  Find: 'F',
  Settings: ',',
  Next: 'Right',
  Prev: 'Left',
  Tools: 'L',
});

const IpcCall: NEnum = enum_num(u8(), {
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
});

const SocketMsg: Enum = enum_lst(u8(), [
  'Unknown',
  'ContentLoaded',
  'KeepAlive',
]);

const StorageId: SEnum = enum_str({
  CurrentView: 'currentView',
  SettingValue: 'someSetting',
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
};

export const PicklersToGenerate: Record<string, Types> = {};
