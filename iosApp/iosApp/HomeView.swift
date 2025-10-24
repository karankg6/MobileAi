import SwiftUI

struct HomeView: View {
    var body: some View {
        VStack(spacing: 20) {
            Image(systemName: "house.fill")
                .font(.system(size: 64))
                .foregroundColor(.blue)

            Text("Home Screen")
                .font(.largeTitle)
                .fontWeight(.bold)

            Text("Welcome to Mobile AI")
                .font(.body)
                .foregroundColor(.secondary)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color(UIColor.systemBackground))
    }
}

struct HomeView_Previews: PreviewProvider {
    static var previews: some View {
        HomeView()
            .preferredColorScheme(.dark)
    }
}
