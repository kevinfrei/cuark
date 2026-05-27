import {
  Button,
  createTableColumn,
  DataGrid,
  DataGridBody,
  DataGridCell,
  DataGridHeader,
  DataGridHeaderCell,
  DataGridProps,
  DataGridRow,
  Dialog,
  DialogBody,
  DialogSurface,
  DialogTrigger,
  makeStyles,
  Menu,
  MenuButton,
  MenuDivider,
  MenuItemCheckbox,
  MenuItemRadio,
  MenuList,
  MenuPopover,
  MenuProps,
  MenuTrigger,
  TableColumnDefinition,
  TableRowId,
} from '@fluentui/react-components';
import {
  ArrowEnterUp20Filled,
  ChevronDown20Filled,
  ChevronLeft20Filled,
  ChevronRight20Filled,
  Options20Regular,
} from '@fluentui/react-icons';
import { isArrayOfString, isNumber } from '@freik/typechk';
import { atom, useAtom, useAtomValue, useSetAtom } from 'jotai';
import {
  ReactElement,
  Suspense,
  use,
  useCallback,
  useEffect,
  useMemo,
  useState,
} from 'react';
import {
  chkFolderContents,
  chkNamedLocations,
  FileSystemItem,
  IpcCall,
  NamedLocations,
} from '../../Shared/CommonTypes';
import { CallMainThrow } from '../../Tools/Ipc';

type FileInfo = FileSystemItem & { viewName: string };

const currentLocationAtom = atom('');
const sidebarVisibleAtom = atom(true);
const itemSelectedAtom = atom('');
const showHiddenAtom = atom(false);
const showFileTypesAtom = atom(true);

const useStyles = makeStyles({
  surface: {
    height: '90vh',
    minWidth: '90vw',
  },
  bodySidebar: {
    height: '100%',
    display: 'grid',
    gridTemplateRows: '30px 1fr 30px',
    gridTemplateColumns: '120px 30px 30px 40px 30px 1fr',
  },
  bodyNoSidebar: {
    height: '100%',
    display: 'grid',
    gridTemplateRows: '30px 1fr 30px',
    gridTemplateColumns: '0px 30px 30px 40px 30px 1fr',
  },
  places: {
    gridRow: '1 / span 3',
    gridColumn: 1,
    overflow: 'clip',
  },
  content: {
    gridRow: '2',
    gridColumn: '2 / span 5',
    overflowX: 'clip',
    overflowY: 'scroll',
    // alignSelf: 'stretch',
    // justifySelf: 'stretch',
  },
  back: {
    gridRow: '1',
    gridColumn: '2',
  },
  fwd: {
    gridRow: '1',
    gridColumn: '3',
  },
  opt: {
    gridRow: '1',
    gridColumn: '4',
    fontSize: 'small',
  },
  up: {
    gridRow: '1',
    gridColumn: '5',
    fontSize: 'small',
  },
  cur: {
    gridRow: '1',
    gridColumn: '6',
    fontSize: 'small',
    margin: '5px',
  },
  actions: {
    display: 'grid',
    gridRow: '3',
    gridColumn: '2 / span 5',
    gridTemplateRows: 'auto',
    gridTemplateColumns: '110px auto 100px 100px',
  },
  new: {
    gridRow: '1',
    gridColumn: '1',
  },
  cancel: {
    gridRow: '1',
    gridColumn: '3',
  },
  select: {
    gridRow: '1',
    gridColumn: '4',
  },
});

let cachedNL: NamedLocations | null = null;
async function GetFavorites(skipCache?: boolean): Promise<NamedLocations> {
  while (cachedNL === null || skipCache) {
    cachedNL = await CallMainThrow(
      IpcCall.GetNamedLocations,
      chkNamedLocations,
    );
    skipCache = false;
  }
  return cachedNL;
}

let cachedRoots: null | string[] = null;
async function GetRootLocations(skipCache?: boolean): Promise<string[]> {
  while (cachedRoots === null || skipCache) {
    cachedRoots = await CallMainThrow(
      IpcCall.GetFileSystemRoots,
      isArrayOfString,
    );
    skipCache = false;
  }
  return cachedRoots;
}

const cachedFC = new Map<string, FileSystemItem[]>();
async function GetFolderContents(
  filePath: string,
  hidden: boolean,
  skipCache?: boolean,
): Promise<FileSystemItem[]> {
  if (filePath.length === 0) {
    const roots = await GetRootLocations();
    return roots.map((val: string) => ({
      file: val,
      date: 0,
      size: -1n,
      type: 'drive',
    }));
  }
  const key = (hidden ? '*' : '+') + filePath;
  let val: undefined | FileSystemItem[] = undefined;
  do {
    if (!skipCache) {
      val = cachedFC.get(key);
    }
    skipCache = false;
    if (!val) {
      val = await CallMainThrow(
        IpcCall.GetFolderContents,
        chkFolderContents,
        filePath,
        !!hidden,
      );
      cachedFC.set(key, val);
    }
  } while (val === undefined);
  return val;
}

function LocationsList(): ReactElement {
  const rootLocPromise = useMemo(() => GetRootLocations(), []);
  const locations = use(rootLocPromise);
  const [curLoc, setCurLoc] = useAtom(currentLocationAtom);
  return (
    <>
      {locations.map((loc) => {
        if (curLoc === loc) {
          return (
            <div key={loc} style={{ fontWeight: 'bold' }}>
              {loc}
            </div>
          );
        } else {
          return (
            <div key={loc} onClick={() => setCurLoc(loc)}>
              {loc}
            </div>
          );
        }
      })}
    </>
  );
}

function FavoritesList(): ReactElement {
  const favPromise = useMemo(() => GetFavorites(), []);
  const favorites = use(favPromise);
  const [curLoc, setCurLoc] = useAtom(currentLocationAtom);
  return (
    <>
      {[...favorites.entries()].map(([key, val]) => {
        if (curLoc === val) {
          return (
            <div key={key} style={{ fontWeight: 'bold' }}>
              {key}
            </div>
          );
        } else {
          return (
            <div key={key} onClick={() => setCurLoc(val)}>
              {key}
            </div>
          );
        }
      })}
    </>
  );
}

type FilePlacesProps = {
  className: string;
};

function FilePlaces({ className }: FilePlacesProps): ReactElement {
  return (
    <div className={className}>
      <div
        key={'locations-header'}
        style={{ fontWeight: 'bold', fontSize: 'larger' }}>
        Locations
      </div>
      <Suspense>
        <LocationsList />
      </Suspense>
      <div
        key="favorites-header"
        style={{ fontWeight: 'bold', fontSize: 'larger', marginTop: '1em' }}>
        Favorites
      </div>
      <Suspense>
        <FavoritesList />
      </Suspense>
    </div>
  );
}

function FileFolderPickerHeader(): ReactElement {
  const classes = useStyles();
  const [curLocVal, setLocation] = useAtom(currentLocationAtom);
  const setSidebarVis = useSetAtom(sidebarVisibleAtom);
  const [hiddenFiles, setHiddenFiles] = useAtom(showHiddenAtom);
  const [showFileTypes, setShowFileTypes] = useAtom(showFileTypesAtom);
  const upDir = useCallback((curLoc: string) => {
    const lastFSlash = curLoc.lastIndexOf('/');
    const lastBSlash = curLoc.lastIndexOf('\\');
    setLocation(
      curLoc.substring(
        0,
        // The third item is to special case the '/' scenario
        Math.max(lastBSlash, lastFSlash, curLoc.length === 1 ? 0 : 1),
      ),
    );
  }, []);
  const show: (string | false)[] = [
    hiddenFiles ? 'hidden' : false,
    showFileTypes ? 'extensions' : false,
  ];
  const [checkedValues, setCheckedValues] = useState<Record<string, string[]>>({
    sidebar: ['visible'],
    show: show.filter((val) => val !== false),
    view: ['list'],
  });
  const onChange: MenuProps['onCheckedValueChange'] = useCallback(
    (e, { name, checkedItems }) => {
      setCheckedValues((s) => {
        return s ? { ...s, [name]: checkedItems } : { [name]: checkedItems };
      });
      if (name === 'sidebar') {
        setSidebarVis(checkedItems.includes('visible'));
      }
      if (name === 'show' && hiddenFiles !== checkedItems.includes('hidden')) {
        setHiddenFiles(checkedItems.includes('hidden'));
      }
      if (
        name === 'show' &&
        showFileTypes !== checkedItems.includes('extensions')
      ) {
        setShowFileTypes(checkedItems.includes('extensions'));
      }
    },
    [showFileTypes, hiddenFiles],
  );
  return (
    <>
      <Button
        className={classes.back}
        icon={<ChevronLeft20Filled />}
        disabled
      />
      <Button
        className={classes.fwd}
        icon={<ChevronRight20Filled />}
        disabled
      />
      <Menu>
        <MenuTrigger>
          <MenuButton
            className={classes.opt}
            icon={
              <>
                <Options20Regular />
                <ChevronDown20Filled />
              </>
            }
            size="large"
          />
        </MenuTrigger>
        <MenuPopover>
          <MenuList
            checkedValues={checkedValues}
            onCheckedValueChange={onChange}>
            <MenuItemCheckbox
              name="sidebar"
              value="visible"
              persistOnClick={false}>
              Show Sidebar
            </MenuItemCheckbox>
            <MenuDivider />
            <MenuItemCheckbox name="show" value="hidden" persistOnClick={false}>
              Show hidden files
            </MenuItemCheckbox>
            <MenuItemCheckbox
              name="show"
              value="extensions"
              persistOnClick={false}>
              Show file extensions
            </MenuItemCheckbox>
            <MenuDivider />
            <MenuItemRadio name="view" value="icons" persistOnClick={false}>
              View as Icons
            </MenuItemRadio>
            <MenuItemRadio name="view" value="list" persistOnClick={false}>
              View as List
            </MenuItemRadio>
          </MenuList>
        </MenuPopover>
      </Menu>
      <Button
        className={classes.up}
        icon={<ArrowEnterUp20Filled />}
        onClick={() => upDir(curLocVal)}
        disabled={curLocVal.length === 0}
      />
      <div className={classes.cur}>{curLocVal}</div>;
    </>
  );
}

function FileFolderPickerFooter(): ReactElement {
  const classes = useStyles();
  const [itemSelected, setItemSelected] = useAtom(itemSelectedAtom);
  return (
    <div className={classes.actions}>
      <Button className={classes.new} appearance="secondary">
        New Folder
      </Button>
      <DialogTrigger disableButtonEnhancement>
        <Button
          className={classes.cancel}
          appearance="secondary"
          onClick={() => setItemSelected('')}>
          Cancel
        </Button>
      </DialogTrigger>
      <DialogTrigger disableButtonEnhancement>
        <Button className={classes.select} disabled={itemSelected === ''}>
          Select
        </Button>
      </DialogTrigger>
    </div>
  );
}

const columns: TableColumnDefinition<FileInfo>[] = [
  createTableColumn<FileInfo>({
    columnId: 'file',
    compare: (a, b) => a.file.localeCompare(b.file),
    renderHeaderCell: () => 'File Name',
    renderCell: (item) => item.viewName,
  }),
  createTableColumn<FileInfo>({
    columnId: 'type',
    compare: (a, b) => a.type.localeCompare(b.type),
    renderHeaderCell: (data) => 'Type',
    renderCell: (item) => item.type,
  }),
  createTableColumn<FileInfo>({
    columnId: 'size',
    compare: (a, b) => (a.size < b.size ? -1 : a.size > b.size ? 1 : 0),
    renderHeaderCell: () => 'File Size',
    renderCell: (item) => (item.type === 'directory' ? '' : item.size),
  }),
  createTableColumn<FileInfo>({
    columnId: 'date',
    compare: (a, b) => a.date - b.date,
    renderHeaderCell: () => 'Date',
    // item.date is millisecondcs since epoch, so we can create a Date object from it and format it as needed
    renderCell: (item) => new Date(item.date).toLocaleString(),
  }),
];

function no_ext(name: string) {
  const lastDot = name.lastIndexOf('.');
  return lastDot > 0 ? name.substring(0, lastDot) : name;
}

function FileFolderPickerContent(): ReactElement {
  const [curLocVal, setCurLoc] = useAtom(currentLocationAtom);
  const showHidden = useAtomValue(showHiddenAtom);
  const showTypes = useAtomValue(showFileTypesAtom);
  const folderContentsPromise = useMemo(
    () => GetFolderContents(curLocVal, showHidden),
    [curLocVal, showHidden],
  );
  const rawData = use(folderContentsPromise);
  const data: FileInfo[] = rawData.map((fsi: FileSystemItem) => ({
    viewName: showTypes ? fsi.file : no_ext(fsi.file),
    ...fsi,
  }));
  const setItemSelected = useSetAtom(itemSelectedAtom);
  const classes = useStyles();
  const defaultSortState = useMemo<
    Parameters<NonNullable<DataGridProps['onSortChange']>>[1]
  >(() => ({ sortColumn: 'file', sortDirection: 'ascending' }), []);
  const [selectedRows, setSelectedRows] = useState(new Set<TableRowId>([]));
  const onSelectionChange: DataGridProps['onSelectionChange'] = (e, d) => {
    setSelectedRows(d.selectedItems);
    if (d.selectedItems.size === 0) {
      setItemSelected('');
    } else {
      const index = d.selectedItems.values().next().value;
      if (isNumber(index)) {
        setItemSelected(data[index].file);
      }
    }
  };

  const columnSizingOptions = {
    display: { minWidth: 150, defaultWidth: 150 },
    size: { minWidth: 10, defaultWidth: 40 },
    date: { minWidth: 20, defaultWidth: 80 },
    type: { minWidth: 20, defaultWidth: 40 },
  };

  return (
    <DataGrid
      className={classes.content}
      items={data}
      columns={columns}
      sortable
      selectionMode="single"
      subtleSelection
      selectedItems={selectedRows}
      onSelectionChange={onSelectionChange}
      defaultSortState={defaultSortState}
      // style={{ minWidth: '500px' }}
      size="small"
      resizableColumns
      resizableColumnsOptions={{ autoFitColumns: false }}
      columnSizingOptions={columnSizingOptions}>
      <DataGridHeader>
        <DataGridRow>
          {({ renderHeaderCell }) => (
            <DataGridHeaderCell>{renderHeaderCell()}</DataGridHeaderCell>
          )}
        </DataGridRow>
      </DataGridHeader>
      <DataGridBody<FileInfo>>
        {({ item, rowId }) => (
          <DataGridRow<FileInfo>
            key={rowId}
            onDoubleClick={() => {
              // If we double-click a folder, navigate to the new folder
              console.log('Dblclk:', curLocVal, item);
              if (item.type === 'directory') {
                // Special-case for a single /
                setCurLoc(
                  curLocVal + (curLocVal.endsWith('/') ? '' : '/') + item.file,
                );
              } else if (item.type === 'drive') {
                setCurLoc(item.file);
              }
            }}>
            {({ renderCell }) => (
              <DataGridCell>{renderCell(item)}</DataGridCell>
            )}
          </DataGridRow>
        )}
      </DataGridBody>
    </DataGrid>
  );
}

type FileFolderPickerProps = {
  onChangeFolder: (path: string) => void;
};

function FileFolderPicker({
  onChangeFolder,
}: FileFolderPickerProps): ReactElement {
  return (
    <>
      <FileFolderPickerHeader />
      <Suspense>
        <FileFolderPickerContent />
      </Suspense>
      <FileFolderPickerFooter />
    </>
  );
}

const DelayedFallback = ({
  delay,
  children,
}: {
  delay: number;
  children: ReactElement;
}) => {
  const [show, setShow] = useState(false);

  useEffect(() => {
    const timer = setTimeout(() => setShow(true), delay);
    return () => clearTimeout(timer);
  }, [delay]);

  return show ? children : null;
};

function FolderChooserSurface(): ReactElement {
  const classes = useStyles();
  const hasSidebar = useAtomValue(sidebarVisibleAtom);
  const setCurLoc = useSetAtom(currentLocationAtom);
  return (
    <DialogSurface className={classes.surface}>
      <DialogBody
        className={hasSidebar ? classes.bodySidebar : classes.bodyNoSidebar}>
        <Suspense>
          {hasSidebar ? <FilePlaces className={classes.places} /> : <></>}
        </Suspense>
        <Suspense>
          <FileFolderPicker onChangeFolder={setCurLoc} />
        </Suspense>
      </DialogBody>
    </DialogSurface>
  );
}

export function FolderChooser(props: {}): ReactElement {
  return (
    <Dialog>
      <DialogTrigger>
        <Button>Open Folder Chooser</Button>
      </DialogTrigger>
      <FolderChooserSurface />
    </Dialog>
  );
}
