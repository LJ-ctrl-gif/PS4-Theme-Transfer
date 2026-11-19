import 'package:flutter_test/flutter_test.dart';
import 'package:ps4_theme_transfer/main.dart';

void main() {
  testWidgets('shows the PS4 Theme Transfer home screen', (tester) async {
    await tester.pumpWidget(const ThemeTransferApp());

    expect(find.text('PS4 THEME TRANSFER'), findsOneWidget);
    expect(find.text('SELECT FILE AND SEND'), findsOneWidget);
    expect(find.text('CHECK CONNECTION'), findsOneWidget);
  });
}
