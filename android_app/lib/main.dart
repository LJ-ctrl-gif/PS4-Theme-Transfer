import 'dart:convert';
import 'dart:io';

import 'package:file_picker/file_picker.dart';
import 'package:flutter/material.dart';

void main() => runApp(const ThemeTransferApp());

class ThemeTransferApp extends StatelessWidget {
  const ThemeTransferApp({super.key});

  @override
  Widget build(BuildContext context) {
    const blue = Color(0xFF38A9FF);
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'PS4 Theme Transfer',
      theme: ThemeData(
        brightness: Brightness.dark,
        colorScheme: ColorScheme.fromSeed(
          seedColor: blue,
          brightness: Brightness.dark,
        ),
        scaffoldBackgroundColor: const Color(0xFF080B10),
        useMaterial3: true,
      ),
      home: const TransferHome(),
    );
  }
}

class TransferHome extends StatefulWidget {
  const TransferHome({super.key});

  @override
  State<TransferHome> createState() => _TransferHomeState();
}

class _TransferHomeState extends State<TransferHome> {
  final _host = TextEditingController();
  String _category = 'pictures';
  String _status = 'Enter the PS4 receiver address to begin.';
  bool _busy = false;
  String? _selectedName;

  static const _categories = <String, String>{
    'pictures': 'Pictures',
    'music': 'Music',
    'sound-effects': 'Sound effects',
    'other': 'Other',
  };

  @override
  void dispose() {
    _host.dispose();
    super.dispose();
  }

  String _baseUrl() {
    var value = _host.text.trim();
    if (value.isEmpty) throw const FormatException('Enter the PS4 IP address.');
    if (!value.startsWith('http://') && !value.startsWith('https://')) {
      value = 'http://$value';
    }
    final uri = Uri.parse(value);
    if (uri.host.isEmpty || uri.userInfo.isNotEmpty) {
      throw const FormatException('Enter a valid local receiver address.');
    }
    return uri.toString().replaceFirst(RegExp(r'/+$'), '');
  }

  Future<void> _checkReceiver() async {
    setState(() {
      _busy = true;
      _status = 'Checking receiver…';
    });
    try {
      final uri = Uri.parse('${_baseUrl()}/api/v1/status');
      final client = HttpClient()..connectionTimeout = const Duration(seconds: 5);
      try {
        final request = await client.getUrl(uri);
        final response = await request.close();
        final body = await response.transform(utf8.decoder).join();
        if (response.statusCode != 200) {
          throw HttpException('Receiver returned HTTP ${response.statusCode}.');
        }
        final data = jsonDecode(body);
        if (data is! Map || data['protocol'] != 1) {
          throw const FormatException('Receiver protocol is not compatible.');
        }
        setState(() => _status = 'Connected to ${data['name'] ?? 'PS4 receiver'}.');
      } finally {
        client.close(force: true);
      }
    } catch (error) {
      setState(() => _status = 'Connection failed: $error');
    } finally {
      if (mounted) setState(() => _busy = false);
    }
  }

  Future<void> _pickAndSend() async {
    if (_busy) return;
    try {
      final result = await FilePicker.platform.pickFiles(
        type: FileType.any,
        allowMultiple: false,
        withData: true,
      );
      if (result == null || result.files.isEmpty) return;
      final file = result.files.single;
      final bytes = file.bytes;
      if (bytes == null || bytes.isEmpty) {
        throw const FormatException('The selected file could not be read.');
      }
      setState(() {
        _busy = true;
        _selectedName = file.name;
        _status = 'Sending ${file.name}…';
      });

      final uri = Uri.parse(
        '${_baseUrl()}/api/v1/upload?category=${Uri.encodeQueryComponent(_category)}',
      );
      final client = HttpClient()..connectionTimeout = const Duration(seconds: 15);
      try {
        final request = await client.postUrl(uri);
        request.headers.contentType = ContentType.binary;
        request.headers.set('X-Filename', Uri.encodeComponent(file.name));
        request.headers.set('Content-Length', bytes.length);
        final response = await request.addStream(Stream.value(bytes)).then((_) => request.close());
        final body = await response.transform(utf8.decoder).join();
        if (response.statusCode < 200 || response.statusCode >= 300) {
          throw HttpException('Upload failed (HTTP ${response.statusCode}): $body');
        }
        setState(() => _status = 'Sent ${file.name} successfully.');
      } finally {
        client.close(force: true);
      }
    } catch (error) {
      setState(() => _status = 'Transfer failed: $error');
    } finally {
      if (mounted) setState(() => _busy = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('PS4 THEME TRANSFER', style: TextStyle(fontWeight: FontWeight.bold)),
        centerTitle: false,
      ),
      body: ListView(
        padding: const EdgeInsets.all(18),
        children: [
          const Icon(Icons.swap_horiz_rounded, size: 62, color: Color(0xFF38A9FF)),
          const SizedBox(height: 8),
          Text('Send theme assets to your PS4',
              style: Theme.of(context).textTheme.headlineSmall?.copyWith(fontWeight: FontWeight.bold)),
          const SizedBox(height: 6),
          const Text('Connect both devices to the same trusted Wi-Fi network.'),
          const SizedBox(height: 22),
          TextField(
            controller: _host,
            keyboardType: TextInputType.url,
            decoration: const InputDecoration(
              labelText: 'PS4 receiver address',
              hintText: 'http://192.168.1.50:8080',
              border: OutlineInputBorder(),
              prefixIcon: Icon(Icons.wifi),
            ),
          ),
          const SizedBox(height: 12),
          OutlinedButton.icon(
            onPressed: _busy ? null : _checkReceiver,
            icon: const Icon(Icons.link),
            label: const Text('CHECK CONNECTION'),
          ),
          const SizedBox(height: 20),
          DropdownButtonFormField<String>(
            value: _category,
            decoration: const InputDecoration(labelText: 'Asset category', border: OutlineInputBorder()),
            items: _categories.entries
                .map((entry) => DropdownMenuItem(value: entry.key, child: Text(entry.value)))
                .toList(),
            onChanged: _busy ? null : (value) => setState(() => _category = value ?? 'pictures'),
          ),
          const SizedBox(height: 14),
          FilledButton.icon(
            onPressed: _busy ? null : _pickAndSend,
            icon: const Icon(Icons.upload_rounded),
            label: const Text('SELECT FILE AND SEND'),
          ),
          if (_busy) ...[
            const SizedBox(height: 18),
            const LinearProgressIndicator(),
          ],
          const SizedBox(height: 18),
          Card(
            child: Padding(
              padding: const EdgeInsets.all(16),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  const Text('TRANSFER STATUS', style: TextStyle(fontWeight: FontWeight.bold)),
                  const SizedBox(height: 8),
                  Text(_status),
                  if (_selectedName != null) ...[
                    const SizedBox(height: 5),
                    Text(_selectedName!, style: Theme.of(context).textTheme.bodySmall),
                  ],
                ],
              ),
            ),
          ),
          const SizedBox(height: 10),
          const Text(
            'Only transfer files you own or are authorized to use. Received files are not installed automatically.',
            style: TextStyle(color: Colors.white60, fontSize: 12),
          ),
        ],
      ),
    );
  }
}
