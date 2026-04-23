import { MakeLog } from '@freik/logger';
import MakeSeqNum from '@freik/seqnum';
import {
  hasFieldType,
  isBoolean,
  isDefined,
  isFunction,
  isNumberOrString,
  isObjectNonNull,
  isString,
  Pickle,
  SafelyUnpickle,
  typecheck,
  Unpickle,
} from '@freik/typechk';
import { chkSocketMsg, IpcCall, SocketMsg } from '../Shared/CommonTypes';
import { ListenKey, MyWindow } from '../Types';

// const { con, log, wrn, err } = MakeLog('tools:ipc');
const con = console.error;
const log = console.error;
const wrn = console.error;
const err = console.error;

declare const window: MyWindow;

/**
 * @async
 * Read a key-value from Electron persistent storage
 *
 * @param key The key to read
 * @returns A promise that resolves to the value read (or void if none found)
 */
export async function ReadFromStorage<T>(
  key: string,
  typechk: typecheck<T>,
): Promise<T | undefined> {
  err(`Reading ${key} from Storage`);
  const res = await CallMain(IpcCall.ReadFromStorage, typechk, key);
  err(`Result from ${key}`, res);
  return res;
}

/**
 * @async
 * Write a key-value pair to Electron persistent storage
 *
 * @param key The key to write
 * @param data The value to be written
 */
export function WriteToStorage<T>(key: string, data: T): void {
  PostMain(IpcCall.WriteToStorage, key, data).catch((err) => {
    err(`Failed to write to storage for key "${key}":`, err);
  });
}

/**
 * @async
 * Delete a key (and it's value...) from Electron persistent storage
 *
 * @param key The key to delete
 */
export function DeleteFromStorage(key: string): void {
  PostMain(IpcCall.DeleteFromStorage, key).catch((err) => {
    err(`Failed to delete from storage for key "${key}":`, err);
  });
}

const getNextListenerSeqNum = MakeSeqNum('Listen');

// map of message names to map of id's to funtions
type Handler<T> = {
  typechk: typecheck<T> | boolean;
  handler: (data: T) => void;
};
const listeners = new Map<SocketMsg, Map<string, Handler<any>>>();

/**
 * This subscribes the `handler` to listen for messages coming from the
 * main process.
 *
 * @param key The message key identified to listen for
 * @param handler The function to invoke upon receipt of the message
 * @returns the key to use to unsubscribe
 */
export function Subscribe<T>(
  ipcId: SocketMsg,
  typechk: typecheck<T>,
  handler: (data: T) => void,
): ListenKey {
  return localSubscribe(ipcId, typechk, handler);
}

function localSubscribe<T>(
  ipcId: SocketMsg,
  typechk: typecheck<T> | boolean,
  handler: (data: T) => void,
): ListenKey {
  const theKey = { ipcId, id: getNextListenerSeqNum() };
  let handlerMap = listeners.get(ipcId);
  if (!handlerMap) {
    handlerMap = new Map<string, Handler<any>>();
    listeners.set(ipcId, handlerMap);
  }
  handlerMap.set(theKey.id, { typechk, handler });
  return theKey;
}

export function SubscribeWithDefault<T>(
  ipcId: SocketMsg,
  typecheck: typecheck<T>,
  handler: (data: T) => void,
  defaultValue: T,
): ListenKey {
  // This is a convenience function that allows you to subscribe to a message
  // and provide a default value to use if the message data does not match
  // the typecheck. This is useful for cases where you want to ensure that
  // the handler always receives a value of the expected type, even if the
  // message data is malformed or missing.
  const defHandler = (data: unknown) => {
    if (typecheck(data)) {
      handler(data);
    } else {
      handler(defaultValue);
    }
  };
  return localSubscribe(ipcId, true, defHandler);
}

export function SubscribeUnsafe(
  ipcId: SocketMsg,
  handler: (data: unknown) => void,
): ListenKey {
  // This is a convenience function that allows you to subscribe to a message
  // and provide a default value to use if the message data does not match
  // the typecheck. This is useful for cases where you want to ensure that
  // the handler always receives a value of the expected type, even if the
  // message data is malformed or missing.
  return localSubscribe(ipcId, true, handler);
}

/**
 * Unsubscribe from a particular message
 *
 * @param listenKey The key returned by {@link Subscribe}
 */
export function Unsubscribe(listenKey: ListenKey): void {
  const lstn = listeners.get(listenKey.ipcId);
  if (lstn) {
    lstn.delete(listenKey.id);
  }
}

let ipcAttempts = 0;

export function WireUpIpc(): void {
  const ws = window.ws;
  if (!ws) {
    ipcAttempts++;
    setTimeout(WireUpIpc, 100);
    if (ipcAttempts % 10 === 0) {
      err(
        `Failed to wire up IPC after ${ipcAttempts} attempts. Please ensure the ReactWebScoket module is loaded.`,
      );
    }
    return;
  }
  err(`Wiring up IPC with WebSocket: ${ws.url}`);
  ws.onmessage = (evt: MessageEvent) => {
    const message = evt.data;
    if (!isString(message)) {
      err(`Received non-string message from websocket:`, message);
      return;
    }
    log(`Received message: ${message}`);
    HandleMessage(message);
  };
  window.ipc = {
    post: (channel: SocketMsg, ...args: unknown[]) => {
      if (!isObjectNonNull(window.ws)) {
        err('IPC connector is not wired up, cannot invoke');
        throw Error('IPC connector is not wired up');
      }
      const msg = `${channel};${Pickle(args)}`;
      log(`Sending message: ${msg}`);
      window.ws.send(msg);
    },
    // TODO: Listen for isDev, maybe?
  };
}

const messageFormat = /^([0-9]+);/;
// Called when an async message comes in from the main process
// I think these should just be subscribed to as part of a useEffect?
function HandleMessage(message: string): void {
  // Walk the list of ID's to see if we've got anything with a format of:
  // { "id" : data }
  // This has an interesting side effect of letting the server process
  // send multiple "messages" in a single message:
  // { artists: ..., albums: ..., songs: ... } will invoke listeners for
  // all three of those 'messages'
  // debugger;
  let handled = false;
  // Messages should come in as a IpcId, a semicolon, then a JSON string
  const match = message.match(messageFormat);
  if (!match || match.length < 2) {
    con('>>> Async malformed message begin');
    con(message);
    con('<<< Async malformed message end');
    return;
  }
  const socketId = Number.parseInt(match[1], 10);
  if (!chkSocketMsg(socketId)) {
    // This is a malformed message, we don't know what to do with it
    con('>>> Async malformed message begin');
    con(`Received message with unknown SocketId: ${socketId}`);
    con(message);
    con('<<< Async malformed message end');
    return;
  }
  const messageData = message.substring(match[0].length);
  const listener = listeners.get(socketId);
  if (!listener) {
    con(`>>> Received message with no listeners: ${socketId}`, message);
    return;
  }
  listener.forEach((hndlr, id) => {
    const { typechk, handler } = hndlr;
    if (isFunction(typechk)) {
      const val = SafelyUnpickle(messageData, typechk);
      if (!isDefined(val)) {
        con(
          `>>> Async malformed message for ${socketId} with id ${id}: `,
          `Failed to unpickle message data:`,
          messageData,
        );
        con('<<< Async malformed message end');
        return;
      }
      handled = true;
      handler(val);
    } else {
      // If we don't have a typechk, we assume the handler can handle anything
      // This is a bit dangerous, but it allows us to have a "catch-all" handler
      // that can handle any message.
      const val = messageData.length > 0 ? Unpickle(messageData) : undefined;
      handled = true;
      handler(val);
    }
    log(`Handling message for ${socketId} with id ${id}`);
  });
  if (!handled) {
    con('**********');
    con(`Unhandled message (SocketMsg: ${socketId})`);
    con(message);
    con('**********');
  }
}

export async function SendMessage<T>(
  channel: SocketMsg,
  ...args: unknown[]
): Promise<unknown> {
  if (!window.ipc) {
    // Wait a bit, as Jotai may call IPC stuff pretty early
    const sleep = (ms: number) => new Promise((r) => setTimeout(r, ms));
    for (let i = 0; i < 20; i++) {
      await sleep(i * i);
      if (window.ipc) {
        break;
      }
    }
  }
  if (!window.ipc) {
    err('InvokeMain called with no IPC connector wired in');
    throw Error('no connector wired in');
  }
  log(`Invoking main("${channel}", "...")`);
  // FIXME: May need a client id of some sort. Not sure...
  let result = await window.ipc.post(channel, ...args);
  log(`Invoke main ("${channel}" "...") returned:`);
  log(result);
  return result;
}

export async function RawGetAsText(
  endpoint: string,
): Promise<string | undefined> {
  try {
    const response = await fetch(endpoint, {
      method: 'GET',
    });
    if (response.ok) {
      return await response.text();
    }
  } catch (e) {
    err(`Failed to fetch ${endpoint}:`, e);
  }
  return undefined;
}

export async function RawGetAsJSON(
  endpoint: string,
): Promise<string | undefined> {
  try {
    const response = await fetch(endpoint, {
      method: 'GET',
    });
    if (response.ok) {
      return await response.json();
    }
  } catch (e) {
    err(`Failed to fetch ${endpoint}:`, e);
  }
  return undefined;
}

// This is designed to be handled on the C++ side automatically,
// so changing stuff here could cause problems. Be careful!
function encodeForCall(arg: unknown): string {
  if (isNumberOrString(arg) || isBoolean(arg)) {
    // Simple stuff goes across as-is, but we still need to encode it for the URL.
    return encodeURIComponent(arg);
  } else if (isObjectNonNull(arg) || Array.isArray(arg)) {
    // Complex stuff gets pickled (mostly just JSON'ed) then encoded for the URL. This allows us to send complex data structures as arguments to IPC calls.
    return encodeURIComponent(Pickle(arg));
  } else {
    throw new Error(
      `Cannot encode argument of type ${typeof arg} for IPC call: ${arg}`,
    );
  }
}

async function Get(endpoint: IpcCall, ...args: unknown[]): Promise<unknown> {
  const path = [endpoint.toString(10), ...args.map(encodeForCall)].join('/');
  log(`Fetching from /api/${path}`);
  let response: Response | undefined;
  try {
    response = await fetch('/api/' + path, { method: 'GET' });
    err('response from /api/${path}', response);
    if (response.ok) {
      const contentType = response.headers.get('Content-Type');
      const isJson = contentType && contentType.includes('json');
      const isText = contentType && contentType.includes('text');
      if (isJson || isText) {
        const txt = await response.text();
        if (txt.length === 0) {
          return undefined;
        }
        return txt;
      } else {
        // log(
        //   `Received non-JSON/text response from ${endpoint}, contentType: ${contentType}`,
        // );
        return await response.blob();
      }
    }
  } catch (e) {
    err(
      `Exception: Failed to fetch ${endpoint} with args: ${args.join(', ')}:`,
      e,
    );
    return {
      error: `Exception: Failed to fetch ${endpoint} with args: ${args.join(', ')}`,
      exception: e,
    };
  }
  return {
    error: `Failed to fetch ${endpoint} with args: ${args.join(', ')}`,
    response,
  };
}

async function GetAs<T>(
  validator: typecheck<T>,
  endpoint: IpcCall,
  ...args: unknown[]
): Promise<T | undefined> {
  const res = await Get(endpoint, ...args);
  if (isString(res)) {
    if (validator(res)) {
      return res;
    }
    try {
      return SafelyUnpickle(res, validator);
    } catch (e) {
      err(
        `Failed to unpickle response from Get(${endpoint}, ${args.join(', ')}):`,
        e,
      );
      err(validator);
      return undefined;
    }
  }
  wrn(`GetAs failed validation from Get(${endpoint}, ${args.join(', ')}):`);
  if (hasFieldType(res, 'text', isFunction)) {
    try {
      const val = await res.text();
      wrn('result.text(): "', val, '"');
      return undefined;
    } catch {}
  }
  wrn(res);
  return undefined;
}

async function Post(endpoint: IpcCall, ...args: unknown[]): Promise<void> {
  try {
    await Get(endpoint, ...args);
  } catch (e) {
    err(`Post failed for (${endpoint}, ${args.join(', ')}):`, e);
  }
}

/**
 * @async
 * Call a remote function with type checking on the return value.
 * If you have no return type, use {@link PostMain} instead.
 *
 * @param channel The channel to send a message to
 * @param typecheck The typecheck function to validate the return type R
 * @param args The data to communicate to the channel (if any)
 * @returns A promise that resolves to the typechecked return value of the RPC
 */
export async function CallMain<T>(
  channel: IpcCall,
  typecheck: typecheck<T>,
  ...args: unknown[]
): Promise<T | undefined> {
  return await GetAs(typecheck, channel, ...args);
}

export async function PostMain(
  channel: IpcCall,
  ...args: unknown[]
): Promise<void> {
  try {
    await Post(channel, ...args);
  } catch (e) {
    // Get the name of the IpcCall, cuz it's useful:
    const key = Object.keys(IpcCall).find(
      (c) => IpcCall[c as keyof typeof IpcCall] === channel,
    );
    err(`Failed PostMain "${key || channel.toString()}":`, e);
    throw e;
  }
}

export function SendMain(channel: IpcCall, ...args: unknown[]): void {
  PostMain(channel, ...args).catch((e) => {
    // Get the name of the IpcCall, cuz it's useful:
    const key = Object.keys(IpcCall).find(
      (c) => IpcCall[c as keyof typeof IpcCall] === channel,
    );
    err(`Failed SendMain "${key || channel.toString()}":`, e);
  });
}
